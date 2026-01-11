/**
 * ============================================================================
 * Keyboard Input Handler
 * ============================================================================
 *
 * Handles button input with debouncing and auto-repeat.
 * Uses ring buffer for key events, scanned from frame callback.
 *
 * Based on CH32LibSDK by Miroslav Nemecek (Panda38)
 * https://github.com/Panda381/CH32LibSDK
 *
 * ============================================================================
 */

#include "../include.h"

#if USE_KEYS

// ============================================================================
// State Variables
// ============================================================================

Bool KeyInitOk = False;
u32 KeyLastPress[KEY_NUM];
u32 KeyLastRelease[KEY_NUM];
volatile Bool KeyPressMap[KEY_NUM];
u8 KeyBuf[KEYBUF_SIZE];
volatile u8 KeyWriteOff = 0;
volatile u8 KeyReadOff = 0;

// GPIO pin mapping - directly indexed by key code (see key.h for pin assignments)
const u8 KeyGpioTab[KEY_NUM] = {
    PC3,    // KEY_RIGHT / KEY_A
    PC2,    // KEY_UP / KEY_B
    PC1,    // KEY_LEFT / KEY_C
};

// write key to keyboard buffer
void KeyWriteKey(u8 key)
{
    u8 w = KeyWriteOff;
    u8 w2 = w + 1;
    if (w2 >= KEYBUF_SIZE) w2 = 0;
    if (w2 != KeyReadOff)
    {
        KeyBuf[w] = key;
        cb();
        KeyWriteOff = w2;
    }
}

// scan keys
void KeyScan()
{
    if (!KeyInitOk) return; // keyboard not initialized

    int i;
    u32 t = Time(); // time in ticks
    for (i = 0; i < KEY_NUM; i++)
    {
        // check if button is pressed
        if (GPIO_In(KeyGpioTab[i]) == 0)
        {
            // button is pressed for the first time
            if (!KeyPressMap[i])
            {
                KeyLastPress[i] = t + (KEYCNT_PRESS - KEYCNT_REPEAT)*1000*HCLK_PER_US;
                KeyPressMap[i] = True;
                KeyWriteKey(i+1);
            }

            // button is already pressed - check repeat interval
            else
            {
                if ((s32)(t - KeyLastPress[i]) >= (s32)(KEYCNT_REPEAT*1000*HCLK_PER_US))
                {
                    KeyLastPress[i] = t;
                    KeyWriteKey(i+1);
                }
            }
            KeyLastRelease[i] = t;
        }

        // button is release - check stop of press
        else
        {
            if (KeyPressMap[i])
            {
                if ((s32)(t - KeyLastRelease[i]) >= (s32)(KEYCNT_REL*1000*HCLK_PER_US))
                {
                    KeyPressMap[i] = False;
                }
            }
        }
    }
}

// get button from keyboard buffer KEY_* (returns NOKEY if no key)
u8 KeyGet()
{
    // check if keyboard buffer is empty
    u8 r = KeyReadOff;
    cb();
    if (r == KeyWriteOff) return NOKEY;

    // get key from keyboard buffer
    u8 ch = KeyBuf[r];

    // write new read offset
    r++;
    if (r >= KEYBUF_SIZE) r = 0;
    KeyReadOff = r;

    return ch;
}

// key flush
void KeyFlush()
{
    while (KeyGet() != NOKEY) {}
}

// check no pressed key
Bool KeyNoPressed()
{
    int i;
    for (i = 0; i < KEY_NUM; i++) if (KeyPressMap[i]) return False;
    return True;
}

// wait for no key pressed
void KeyWaitNoPressed()
{
    while (!KeyNoPressed()) {}
}

// Initialize keyboard service (should be called before display init)
void KeyInit()
{
    KeyWriteOff = 0;
    KeyReadOff = 0;

    // initialize key ports
    RCC_PCClkEnable();
    int i;
    for (i = 0; i < KEY_NUM; i++)
    {
        GPIO_Mode(KeyGpioTab[i], GPIO_MODE_IN_PU);
        GPIO_Out1(KeyGpioTab[i]);
        KeyPressMap[i] = False;
    }

    KeyInitOk = True; // keyboard initialized
}

// terminate keyboard
void KeyTerm()
{
    KeyInitOk = False; // keyboard not initialized
    cb();
    int i;
    for (i = 0; i < KEY_NUM; i++) GPIO_PinReset(KeyGpioTab[i]);
}

#endif // USE_KEY
