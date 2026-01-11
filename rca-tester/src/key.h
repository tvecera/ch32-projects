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
 * Hardware:
 * - PC1: Button C / LEFT
 * - PC2: Button B / UP
 * - PC3: Button A / RIGHT
 *
 * ============================================================================
 */

#if USE_KEYS

#ifndef _RCA_KEY_H
#define _RCA_KEY_H

#ifdef __cplusplus
extern "C" {
#endif

// Button codes (order corresponds to the order of the arrow characters in the font)
#define KEY_A   3   // right (character code 0x0C = arrow right, or ASCII code 0x1C)
#define KEY_B   2   // up (character code 0x0D = arrow up, or ASCII code 0x1D)
#define KEY_C   1   // left (character code 0x0E = arrow left, or ASCII code 0x1E)

#define KEY_RIGHT   3
#define KEY_UP      2
#define KEY_LEFT    1

#define NOKEY       0   // no key

#define KEY_NUM     3   // number of buttons

#define KEYCNT_REL      50  // keyboard counter - release interval in [ms]
#define KEYCNT_PRESS    400 // keyboard counter - first repeat in [ms]
#define KEYCNT_REPEAT   100 // keyboard counter - next repeat in [ms]

// ============================================================================
// Ring Buffer State
// ============================================================================

#define KEYBUF_SIZE 8

extern volatile Bool KeyPressMap[KEY_NUM];
extern u8 KeyBuf[KEYBUF_SIZE];
extern volatile u8 KeyWriteOff;
extern volatile u8 KeyReadOff;

// ============================================================================
// Public API
// ============================================================================

/**
 * @brief Check if a button is currently pressed
 * @param key Button code (KEY_A, KEY_B, KEY_C or KEY_LEFT, KEY_UP, KEY_RIGHT)
 * @return True if button is pressed, False otherwise
 */
INLINE Bool KeyPressed(u8 key) { return KeyPressMap[key - 1]; }

/**
 * @brief Scan keyboard for button presses
 *
 * Should be called periodically (from frame callback or timer ISR).
 * Handles debouncing and auto-repeat timing.
 */
void KeyScan();

/**
 * @brief Get next key from keyboard buffer
 * @return Button code (KEY_*) or NOKEY if buffer is empty
 */
u8 KeyGet();

/**
 * @brief Flush keyboard buffer
 *
 * Discards all pending key events.
 */
void KeyFlush();

/**
 * @brief Check if no buttons are pressed
 * @return True if all buttons are released
 */
Bool KeyNoPressed();

/**
 * @brief Wait until all buttons are released
 *
 * Blocks until no button is pressed.
 */
void KeyWaitNoPressed();

/**
 * @brief Initialize keyboard service
 *
 * Configures GPIO pins for button input with internal pull-ups.
 * Should be called before starting video output.
 */
void KeyInit();

/**
 * @brief Terminate keyboard service
 *
 * Resets GPIO pins to default state.
 */
void KeyTerm();

#ifdef __cplusplus
}
#endif

#endif // _RCA_KEY_H

#endif // USE_KEYS
