// ****************************************************************************
//
//                             RVPC - PS/2 Keyboard
//
// ****************************************************************************

#if USE_KEYS

#ifndef _RVPC_KEY_H
#define _RVPC_KEY_H

#ifdef __cplusplus
extern "C" {

#endif

// Button codes (must match bit positions!)
#define KEY_RIGHT	1	// bit 0
#define KEY_UP		2	// bit 1
#define KEY_LEFT	3	// bit 2
#define KEY_DOWN	4	// bit 3
#define KEY_X		5	// bit 4
#define KEY_Y		6	// bit 5
#define KEY_A		7	// bit 6
#define KEY_B		8	// bit 7

#define NOKEY		0

#define KEY_NUM		8

// keyboard buffer
#define KEYBUF_SIZE	16

#if USE_RCA
// RCA version - C implementation with EXTI
// One queued PS/2 event captured on falling clock edge
typedef struct {
    u8 scancode;
    u8 flags;
} PS2KeyEvent;

#define PS2_BUF_SIZE 16
#define PS2_KEY_MAKE     0x00
#define PS2_KEY_BREAK    0x80
#define PS2_KEY_EXTENDED 0x40

#define PS2_STATE_IDLE 0
#define PS2_STATE_E0   1
#define PS2_STATE_F0   2
#define PS2_STATE_E0F0 3

extern PS2KeyEvent PS2Buffer[PS2_BUF_SIZE];
extern volatile u8 PS2ReadIdx;
extern volatile u8 PS2WriteIdx;
extern volatile u32 PS2ShiftReg;
extern volatile u8 PS2BitCount;
extern volatile u8 PS2State;
#else
// VGA version - Assembly implementation
extern volatile u32 PS2_Data[2];
extern volatile u8 PS2_BitCount;
extern volatile u8 PS2_PrevClk;
extern volatile u8 PS2_ExtendedBreakFlag;
#endif

// Current state of all keys - ONE BYTE!
// Bit 0 = KEY_RIGHT, Bit 1 = KEY_UP, etc.
extern volatile u8 KeyState;

extern volatile u16 KeyBuf[KEYBUF_SIZE];
extern volatile u8 KeyWriteOff;
extern volatile u8 KeyReadOff;

// check if button KEY_* is currently pressed
INLINE Bool KeyPressed(int key) {
    if (key < 1 || key > KEY_NUM) return False;
    return (KeyState & (1 << (key - 1))) != 0;
}

// get button from keyboard KEY_* (returns NOKEY if no key)
u8 KeyGet();

// check if joystick is pressed (KEY_RIGHT, KEY_UP, KEY_LEFT, KEY_DOWN, KEY_A, KEY_B, KEY_X, KEY_Y)
Bool JoyPressed(int key);

// get joystick (returns NOKEY or KEY_RIGHT, KEY_UP, KEY_LEFT, KEY_DOWN, KEY_A, KEY_B, KEY_X, KEY_Y)
u8 JoyGet();

// get raw scancode (for debugging)
u16 KeyGetRaw();

// convert scancode to ASCII character
u8 KeyGetChar(u32 scancode);

// key flush
void KeyFlush();

// check no pressed key
INLINE Bool KeyNonePressed() {
    return KeyState == 0;
}

void KeyWaitPressed();

// wait for no key pressed
void KeyWaitNoPressed();

// Initialize keyboard service
void KeyInit();

// terminate keyboard
void KeyTerm();

// Debug functions
void KeyDebugPrintBuffer();
u8 KeyDebugBufferCount();
void KeyDebugPrintStatus();

#ifdef __cplusplus
}
#endif

#endif // _RVPC_KEY_H

#endif // USE_KEYS
