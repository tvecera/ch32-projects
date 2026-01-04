/**
 * MIT License
 *
 * Copyright (c) 2026 Tomas Vecera, tomas@vecera.dev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ============================================================================
 * RCA Composite Video Driver for CH32V002/CH32V003
 * ============================================================================
 *
 * PAL-like composite video output using bit-banging on GPIO pins.
 * TIM2 generates HSYNC pulses via PWM, TIM2 CC2 interrupt triggers pixel output.
 *
 * Hardware connections (defaults, see rca_video.c for details):
 *   PD6 -> 330R -> VIDEO (RCA center)
 *   PD4 -> 680R -> SYNC (combined with VIDEO)
 *   GND -> GND (RCA shield)
 *
 * Timing: 312 lines/frame, ~50 Hz, ~15.65 kHz line rate
 *
 * Video modes summary:
 * +-------+-----------+---------+-------+-------+----------+-------------+
 * | VMODE | Type      | Size    | FBuf  | State | Handler  | Total RAM   |
 * +-------+-----------+---------+-------+-------+----------+-------------+
 * |   0   | Graphics  | 128x64  | 1024B |   -   |   306B   |    1330B    |
 * |   1   | Graphics  | 160x120 | 2400B |   -   |   330B   |    2730B    |
 * |   6   | Text 8x8  | 40x30   | 1200B |  20B  |   376B   |    1596B    |
 * |   7   | Text 8x8  | 32x24   | 768B  |  20B  |   430B   |    1218B    |
 * |   8   | Text 8x14 | 23x18   | 414B  |  64B  |   418B   |     896B    |
 * +-------+-----------+---------+-------+-------+----------+-------------+
 *
 * Notes:
 * - Some function and variable names are taken from CH32LibSDK (https://github.com/Panda381/CH32LibSDK)
 * - CH32V002 has flash wait states and less stable HSI oscillator compared to
 *   CH32V003, causing more visible image jitter/wobble when running on HSI only.
 *   For best RCA output quality, prefer CH32V002 with external HSE crystal.
 *
 * ============================================================================
 */
#ifndef _RCA_H
#define _RCA_H

#ifdef __cplusplus
extern "C" {
#endif

#if VMODE == 0
#include "rca_vmode_0.h"
#elif VMODE == 1
#include "rca_vmode_1.h"
#elif VMODE == 6
#include "rca_vmode_6.h"
#elif VMODE == 7
#include "rca_vmode_7.h"
#elif VMODE == 8
#include "rca_vmode_8.h"
#else
#error "Unsupported VMODE"
#endif

extern u8 FrameBuf[FRAMESIZE]; // display frame buffer (text chars or graphics pixels)
extern volatile u32 DispLine;  // current scanline (0..311), updated by IRQ handler
extern volatile u32 DispFrame; // frame counter, incremented each VSYNC

/**
 * Check if display is in VSYNC period (outside active video area)
 * @return non-zero if in VSYNC, 0 if in active video area
 */
INLINE static u8 rca_in_vsync() {
    return DispLine >= ACTIVE_LINES;
}

/**
 * Block until next VSYNC period starts
 * Use before updating frame buffer to prevent tearing
 */
INLINE static void rca_wait_vsync() {
    while (rca_in_vsync()) {}
    while (!rca_in_vsync()) {}
}

/**
 * Get current frame counter
 * Incremented at each VSYNC (every ~20ms at 50Hz)
 * @return Frame count since rca_init()
 */
INLINE static u32 rca_get_frame_count() {
    return DispFrame;
}

/**
 * Get current scanline number
 * @return Current line (0..311), updated by IRQ handler
 */
INLINE static u32 rca_get_current_line() {
    return DispLine;
}

/**
 * Check if display is in active video area
 * @return non-zero if rendering pixels, 0 if in blanking
 */
INLINE static u8 rca_in_active_video() {
    return DispLine < ACTIVE_LINES;
}

/**
 * Initialize RCA composite video output
 *
 * Configures:
 *   - HSI oscillator trim for ~50 MHz operation
 *   - GPIO: SYNC_PIN as TIM2_CH1 PWM output for HSYNC
 *   - GPIO: VIDEO_PIN as video data output (bit-bang or SPI)
 *   - TIM2: PWM mode for HSYNC pulses, CC2 interrupt for pixel rendering
 *   - NVIC: Enable TIM2 interrupt
 */
void rca_init();

/**
 * Terminate RCA composite video output
 *
 * Disables TIM2 interrupts and timer, resets GPIO pins to low,
 * and restores the HSI trim to its default value.
 */
void rca_stop();

/**
 * Set frame end callback function
 *
 * Called from IRQ context at the end of each frame (during VBLANK period).
 *
 * @param callback Function pointer to call at frame end, or 0 to disable
 *
 * @warning Callback must complete within VBLANK period!
 *          Maximum allowed duration: ~1.5ms (PAL VBLANK = 25 lines * 64us)
 *          Typical safe limit: 1.0ms to allow for timing jitter
 *
 * @note Callback is called from IRQ context with interrupts disabled.
 *       Do not use blocking operations, heavy computation, or call
 *       functions that may take variable time. Suitable for:
 *       - Keyboard scanning (KeyScan)
 *       - Sound buffer updates (SoundScan)
 *       - Simple state machine updates
 */
void rca_set_frame_callback(void (*callback)());

/**
 * Update font rendering state for text modes.
 *
 * Call this function after changing the global font pointer (`DrawFont` or `fontdata`)
 * to ensure the new font is applied correctly at the next frame.
 *
 * This function resets internal state variables used for text rendering,
 * including pointers to the font data. It should be called outside of
 * the frame rendering loop to avoid visual artifacts.
 *
 * @note This function only has an effect in text modes (VMODE 6, 7, 8).
 *       It does nothing in graphical modes.
 */
void rca_update_font();

#ifdef __cplusplus
}
#endif

#endif // _RCA_H
