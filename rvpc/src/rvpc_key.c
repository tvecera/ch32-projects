// ****************************************************************************
//
//                      RVPC - PS/2 Keyboard Support
//
// ****************************************************************************
#include INCLUDES_H     // all includes
#include "_include.h"

#if USE_KEYS

// ONE BYTE for all key states!
volatile u8 KeyState = 0;

volatile u16 KeyBuf[KEYBUF_SIZE];
volatile u8 KeyWriteOff = 0;
volatile u8 KeyReadOff = 0;

#if USE_RCA
// ============================================================================
//                   RCA Version - C Implementation with EXTI
// ============================================================================

// EXTI-driven PS/2 receiver: collects raw bytes then translates to the legacy KeyBuf interface.
PS2KeyEvent PS2Buffer[PS2_BUF_SIZE];
volatile u8 PS2ReadIdx = 0;
volatile u8 PS2WriteIdx = 0;

// State machine for PS/2 protocol
volatile u32 PS2ShiftReg = 0;
volatile u8 PS2BitCount = 0;
volatile u8 PS2State = PS2_STATE_IDLE;

#else
// ============================================================================
//                   VGA Version - Assembly Implementation
// ============================================================================

volatile u32 PS2_Data[2] = {0};
volatile u8 PS2_BitCount = 0;
volatile u8 PS2_PrevClk = 1;
volatile u8 PS2_ExtendedBreakFlag = 0;
#endif // USE_RCA

static const u8 ScanCodes[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, '`', 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 'q', '1', 0x00, 0x00, 0x00, 'z', 's', 'a', 'w', '2', 0x00,
    0x00, 'c', 'x', 'd', 'e', '4', '3', 0x00, 0x00, ' ', 'v', 'f', 't', 'r', '5', 0x00,
    0x00, 'n', 'b', 'h', 'g', 'y', '6', 0x00, 0x00, 0x00, 'm', 'j', 'u', '7', '8', 0x00,
    0x00, ',', 'k', 'i', 'o', '0', '9', 0x00, 0x00, '.', '/', 'l', ';', 'p', '-', 0x00,
    0x00, 0x00, '\'', 0x00, '[', '=', 0x00, 0x00, 0x00, 0x00, 0x0D, ']', 0x00, '\\', 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
};

// Map scancode to KEY_*
static u8 ScanToKey(const u16 scancode) {
    const u8 low = scancode & 0xFF;
    const u8 high = (scancode >> 8) & 0xFF;

    if (high == 0x00) {
        switch (low) {
            case 0x1C:
            case 0x29:
            case 0x5A: return KEY_A;
            case 0x32: return KEY_B;
            case 0x22: return KEY_X;
            case 0x35: return KEY_Y;
            default: return NOKEY;
        }
    }

    if (high == 0xE0) {
        switch (low) {
            case 0x6B: return KEY_LEFT;
            case 0x74: return KEY_RIGHT;
            case 0x75: return KEY_UP;
            case 0x72: return KEY_DOWN;
            default: return NOKEY;
        }
    }

    return NOKEY;
}

u8 KeyGetChar(const u32 scancode) {
    if (scancode == NOKEY) return NOKEY;
    const u8 index = (u8)(scancode & 0xFF);
    if (index < sizeof(ScanCodes)) {
        return ScanCodes[index];
    }
    return NOKEY;
}

u8 KeyGet() {
    u16 scancode;
    u8 key;
    int loop_count = 0;

    while (loop_count < KEYBUF_SIZE) {
        loop_count++;

        if (KeyReadOff == KeyWriteOff) {
            return NOKEY;
        }

        scancode = KeyBuf[KeyReadOff];

        KeyReadOff++;
        if (KeyReadOff >= KEYBUF_SIZE) {
            KeyReadOff = 0;
        }

        key = ScanToKey(scancode);

        // Return only make codes (break codes already processed in ASM)
        if (key != NOKEY) {
            return key;
        }
    }

    return NOKEY;
}

// check if joystick is pressed (KEY_RIGHT, KEY_UP, KEY_LEFT, KEY_DOWN, KEY_A, KEY_B, KEY_X, KEY_Y)
Bool JoyPressed(const int key) {
    switch (key) {
        case KEY_RIGHT:
            return KeyPressed(KEY_RIGHT);
        case KEY_UP:
            return KeyPressed(KEY_UP);
        case KEY_LEFT:
            return KeyPressed(KEY_LEFT);
        case KEY_DOWN:
            return KeyPressed(KEY_DOWN);
        case KEY_A:
            return KeyPressed(KEY_A);
        case KEY_B:
            return KeyPressed(KEY_B);
        case KEY_X:
            return KeyPressed(KEY_X);
        case KEY_Y:
            return KeyPressed(KEY_Y);
        default:
            return False;
    }
}

// get joystick (returns NOKEY or KEY_RIGHT, KEY_UP, KEY_LEFT, KEY_DOWN, KEY_A, KEY_B, KEY_X, KEY_Y)
u8 JoyGet() {
    u8 key;
    while (True) {
        key = KeyGet();
        switch (key) {
            case KEY_RIGHT:
                return KEY_RIGHT;
            case KEY_UP:
                return KEY_UP;
            case KEY_LEFT:
                return KEY_LEFT;
            case KEY_DOWN:
                return KEY_DOWN;
            case KEY_A:
                return KEY_A;
            case KEY_B:
                return KEY_B;
            case KEY_X:
                return KEY_X;
            case KEY_Y:
                return KEY_Y;
            case NOKEY:
                return NOKEY;
        }
    }
}

u16 KeyGetRaw() {
    if (KeyReadOff == KeyWriteOff) return NOKEY;

    const u16 scancode = KeyBuf[KeyReadOff];

    KeyReadOff++;
    if (KeyReadOff >= KEYBUF_SIZE) {
        KeyReadOff = 0;
    }

    return scancode;
}

void KeyFlush() {
    KeyWriteOff = 0;
    KeyReadOff = 0;
    KeyState = 0;

#if USE_RCA
    PS2ReadIdx = 0;
    PS2WriteIdx = 0;
    PS2BitCount = 0;
    PS2ShiftReg = 0;
    PS2State = PS2_STATE_IDLE;
#else
    PS2_Data[0] = 0;
    PS2_Data[1] = 0;
    PS2_BitCount = 0;
    PS2_PrevClk = 1;
#endif

    WaitMs(100);
}

void KeyWaitNoPressed() {
    while (KeyState != 0) {
        WaitMs(10);
    }
}

void KeyWaitPressed() {
    while (KeyNonePressed()) {
    }
}

#if USE_RCA
// ============================================================================
//                   RCA Version - EXTI Interrupt Handler
// ============================================================================

// Add key event to buffer (drops oldest when full to keep state up to date)
NOINLINE void PS2PutKey(u8 scancode, u8 flags) {
    // Update KeyState
    u16 fullcode;
    if (flags & PS2_KEY_EXTENDED) {
        if (flags & PS2_KEY_BREAK) {
            fullcode = 0xE100 | scancode;  // Extended break: E1xx
        } else {
            fullcode = 0xE000 | scancode;  // Extended make: E0xx
        }
    } else {
        if (flags & PS2_KEY_BREAK) {
            fullcode = 0xF000 | scancode;  // Normal break: F0xx
        } else {
            fullcode = scancode;           // Normal make: 00xx
        }
    }

    u16 key_lookup = (flags & PS2_KEY_EXTENDED) ? (0xE000 | scancode) : scancode;
    u8 key = ScanToKey(key_lookup);

    // Update KeyState based on MAKE or BREAK
    if (key != NOKEY) {
        if (flags & PS2_KEY_BREAK) {
            KeyState &= ~(1u << (key - 1));  // Clear bit on key release
        } else {
            KeyState |= (1u << (key - 1));   // Set bit on key press
        }
    }

    // Store to PS2Buffer
    u8 next_wr = (PS2WriteIdx + 1) & (PS2_BUF_SIZE - 1);
    if (next_wr == PS2ReadIdx) {
        PS2ReadIdx = (PS2ReadIdx + 1) & (PS2_BUF_SIZE - 1);
    }
    PS2Buffer[PS2WriteIdx].scancode = scancode;
    PS2Buffer[PS2WriteIdx].flags = flags;
    PS2WriteIdx = next_wr;

    // Store to KeyBuf - stores ALL scancodes including break codes
    u8 next = (KeyWriteOff + 1) & (KEYBUF_SIZE - 1);
    if (next != KeyReadOff) {
        KeyBuf[KeyWriteOff] = fullcode;
        KeyWriteOff = next;
    }
}

// EXTI interrupt handler (EXTI0-7)
#if CH32V003
__attribute__((interrupt)) void EXTI7_0_IRQHandler(void);
#else
__attribute__((interrupt)) void NOFLASH(EXTI7_0_IRQHandler)(void);
#endif

void EXTI7_0_IRQHandler(void) {
    // Check if EXTI1 triggered (PD1 - PS/2 CLK)
    u32 intfr = EXTI->INTFR;
    if (!(intfr & 2)) return;
    EXTI->INTFR = 2;

    u32 reg = PS2ShiftReg;
    u32 count = PS2BitCount;

    // Read DATA pin and shift into register
    reg = (reg >> 1) | ((GPIOA->INDR & (1 << 2)) ? 0x400 : 0);

    if (++count < 11) {
        PS2ShiftReg = reg;
        PS2BitCount = count;
        return;
    }

    // Reset counters
    PS2BitCount = 0;
    PS2ShiftReg = 0;

    if (count > 11) {
        PS2State = PS2_STATE_IDLE;
        return;
    }

    // Validate frame: start=0, stop=1
    if ((reg ^ 0x400) & 0x401) {
        PS2State = PS2_STATE_IDLE;
        return;
    }

    // Extract data byte (bits 1-8)
    u8 data = (u8)(reg >> 1);
    u8 state = PS2State;

    // State 0 (IDLE): check for E0/F0 prefix
    if (state == PS2_STATE_IDLE) {
        if (data < 0xE0) {
            PS2PutKey(data, PS2_KEY_MAKE);
            return;
        }
        PS2State = (data == 0xE0) ? 1 : 2;
        return;
    }

    // State 1 (E0): extended key
    if (state == 1) {
        if (data == 0xF0) {
            PS2State = 3;
            return;
        }
        PS2State = 0;
        PS2PutKey(data, PS2_KEY_MAKE | PS2_KEY_EXTENDED);
        return;
    }

    // State 2 (F0): break code
    // State 3 (E0F0): extended break
    PS2State = 0;
    PS2PutKey(data, PS2_KEY_BREAK | ((state & 1) ? PS2_KEY_EXTENDED : 0));
}

void KeyInit() {
    RCC_PAClkEnable();
    RCC_PDClkEnable();
    RCC_AFIClkEnable();

    // PA2 - DATA (input with pull-up)
    GPIO_Mode(PA2, GPIO_MODE_IN_PU);

    // PD1 - CLK (input with pull-up)
    GPIO_Mode(PD1, GPIO_MODE_IN_PU);

    // Configure EXTI on PD1 (falling edge)
    GPIO_EXTILine(PORTD_INX, 1);
    EXTI_FallEnable(1);
    EXTI_Enable(1);

    NVIC_IRQEnable(IRQ_EXTI7);

    // Clear state
    KeyFlush();
}

void KeyTerm() {
    NVIC_IRQDisable(IRQ_EXTI7);
    KeyFlush();
    GPIO_PinReset(PA2);
    GPIO_PinReset(PD1);
}

#else
// ============================================================================
//                   VGA Version - Original Implementation
// ============================================================================

void KeyInit() {
    RCC_PAClkEnable();
    RCC_PDClkEnable();

    GPIO_Mode(PA2, GPIO_MODE_IN);
    GPIO_Out0(PD1);
    GPIO_Mode(PD1, GPIO_MODE_IN);

    KeyFlush();
}

void KeyTerm() {
    KeyFlush();
    GPIO_PinReset(PA2);
    GPIO_PinReset(PD1);
}

#endif // USE_RCA

#endif // USE_KEYS
