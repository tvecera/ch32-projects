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
 * RCA Composite Video Library (PAL-like timing)
 * ============================================================================
 *
 * Generates composite video signal using bit-banging on GPIO pins.
 * SYNC_PIN outputs SYNC via TIM2 PWM, VIDEO_PIN outputs VIDEO data via IRQ.
 *
 * PIN ASSIGNMENT:
 * ----------------
 *  VIDEO_PIN (PD6) - video data output (bit-banged in IRQ handler)
 *  SYNC_PIN  (PD4) - sync signal output (TIM2_CH1 PWM)
 *
 * CLOCK CONFIGURATION:
 * --------------------
 * HSI Oscillator: ~24 MHz default, trimmed to ~50 MHz via RCC_HSITrim(31)
 * Timer clock: ~50 MHz (HSI is not crystal-accurate, expect ±2% variation)
 *
 * LINE TIMING (at 50 MHz):
 * ------------------------
 * Line period:  3194 ticks = 63.88 us (PAL target: 64 us)
 * Line rate:    50 MHz / 3194 = 15.65 kHz (PAL target: 15.625 kHz)
 * HSYNC pulse:  235 ticks = 4.7 us (PAL target: 4.7 us)
 * Video start:  560 ticks = 11.2 us (after HSYNC + back porch)
 *
 * PERIOD CALIBRATION:
 * -------------------
 * Theoretical ideal: 3200 (for exactly 15.625 kHz at 50 MHz)
 * Actual value: 3194 (empirically tuned for stable sync)
 *
 * Each MCU may vary - adjust TIM_LINE_PERIOD if display shows rolling sync:
 *   - Line rate too low (< 15.625 kHz): decrease period
 *   - Line rate too high (> 15.650 kHz): increase period
 *
 * FRAME TIMING:
 * -------------
 * Total lines: 312 (PAL: 312.5)
 * Frame rate:  15.65 kHz / 312 = 50.16 Hz (PAL target: 50 Hz)
 * VSYNC:       lines 302-310 (long HSYNC pulse)
 * Active area: depends on VMODE
 *
 * HARDWARE WIRING (5V supply):
 * ------------------------------
 *  VIDEO_PIN (PD6 / PC6 (SPI)) -> 330R  -> VIDEO (RCA center)
 *  SYNC_PIN  (PD4)             -> 680R  -> VIDEO (combined with video)
 *  GND                         -> GND (RCA shield)
 *
 * Signal levels (resistor divider into 75R TV load):
 *  - Sync:  ~0.0V (SYNC_PIN low, VIDEO_PIN low)
 *  - Black: ~0.3V (SYNC_PIN high, VIDEO_PIN low)
 *  - White: ~1.0V (SYNC_PIN high, VIDEO_PIN high)
 *
 * ============================================================================
 * CHANGING PINS - CHECKLIST
 * ============================================================================
 *
 * To change VIDEO_PIN or SYNC_PIN, you must update:
 *
 * 1. PIN DEFINITIONS (in "Pin Configuration" section below):
 *    - VIDEO_PIN      - GPIO pin for video data (e.g., PD6, PC6)
 *    - VIDEO_PIN_MASK - Bit mask matching the pin number (e.g., 1 << 6)
 *    - SYNC_PIN       - GPIO pin for sync signal (must support TIM2_CH1)
 *
 * 2. GPIO PORT CLOCKS (in rca_init):
 *    - Enable clock for the port(s) used: RCC_PAClkEnable(), RCC_PBClkEnable(),
 *      RCC_PCClkEnable(), RCC_PDClkEnable()
 *
 * 3. TIM2 REMAPPING (in rca_init):
 *    - SYNC_PIN must be a valid TIM2_CH1 output pin
 *    - Call GPIO_Remap_TIM2(n) with correct remap value for your pin:
 *
 *      CH32V002/V006/V007 (3-bit remap):
 *        0: TIM2_CH1 = PD4    1: TIM2_CH1 = PC1    2: TIM2_CH1 = PC5
 *        3: TIM2_CH1 = PC1    4: TIM2_CH1 = PC0    5: TIM2_CH1 = PA0
 *        6: TIM2_CH1 = PB1    7: TIM2_CH1 = PD3
 *
 *      CH32V003 (2-bit remap):
 *        0: TIM2_CH1 = PD4    1: TIM2_CH1 = PC5
 *        2: TIM2_CH1 = PC1    3: TIM2_CH1 = PC1
 *
 * 4. DIRECT REGISTER ACCESS (if VIDEO_PIN port changes):
 *    - Update GPIOD_BCR to match the new port (e.g., GPIOC_BCR for port C)
 *    - Update gpiod_data address in SendByteBitBang() assembly functions
 *      (GPIOD_BASE + 0x0C for port D, GPIOC_BASE + 0x0C for port C, etc.)
 *
 * ============================================================================
 */
#include "../include.h"

// ============================================================================
// Global State Variables
// ============================================================================
u8 FrameBuf[FRAMESIZE];         // Frame buffer (text chars or graphics pixels)
volatile u32 DispLine = 0;      // Current scanline (0..311), updated by IRQ
volatile u32 DispFrame = 0;     // Frame counter, incremented at each VSYNC

// Frame end callback function pointer
typedef void (*rca_frame_callback_t)();
static rca_frame_callback_t frame_end_callback = 0;

// ============================================================================
// Timer Constants (at 50MHz SYSCLK)
// ============================================================================
#ifndef TIM2_CLK_HZ
#define TIM2_CLK_HZ         50000000UL // Adjust to measured timer clock (e.g. HSI/PLL drift on V002)
#endif

#define TIM_TICKS_FROM_NS(ns) ((u32)((((u64)TIM2_CLK_HZ) * (ns) + 500000000ULL) / 1000000000ULL))

#define TIM_LINE_PERIOD     TIM_TICKS_FROM_NS(63880ULL)  // Line period: ~63.88us (~15.65 kHz)
#define TIM_HSYNC_PULSE     TIM_TICKS_FROM_NS(4700ULL)   // HSYNC pulse: ~4.7us (PAL standard)
#define TIM_VSYNC_LONG      TIM_TICKS_FROM_NS(32000ULL)  // VSYNC pulse: ~32us (long HSYNC)

#ifndef TIM_VIDEO_START
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(11200ULL)  // Video start: ~11.2us (after back porch)
#endif

// Frame structure constants
#define TOTAL_LINES         312     // Total lines per frame (PAL: 312.5)
#define VSYNC_START_LINE    302     // First line of VSYNC period
#define VSYNC_END_LINE      310     // Last line of VSYNC period

// ============================================================================
// Pin Configuration
// ============================================================================
// VIDEO_PIN: Direct bit-bang output for pixel data (or SPI MOSI for SPI mode)
// SYNC_PIN:  TIM2_CH1 PWM output for HSYNC/VSYNC pulses

#if USE_RCA_SPI
// SPI mode: PC6 = SPI1_MOSI (video data via hardware SPI)
#define VIDEO_PIN           PC6         // Video data output pin (SPI1_MOSI)
#define VIDEO_PIN_MASK      (1 << 6)    // Bit mask for VIDEO_PIN in GPIOC
#define VIDEO_GPIO_BCR      (*(volatile u32*)(GPIOC_BASE + 0x14)) // GPIOC Bit Clear Register
#else
// Bit-bang mode: PD6 = GPIO output (video data via software bit-banging)
#define VIDEO_PIN           PD6         // Video data output pin
#define VIDEO_PIN_MASK      (1 << 6)    // Bit mask for VIDEO_PIN in GPIOD
#define VIDEO_GPIO_BCR      (*(volatile u32*)(GPIOD_BASE + 0x14)) // GPIOD Bit Clear Register
#endif

#if USE_RVPC
#define SYNC_PIN            PC1         // Sync signal output pin (TIM2_CH1)
#else
#define SYNC_PIN            PD4         // Sync signal output pin (TIM2_CH1)
#endif

// Legacy alias for backward compatibility
#define GPIOD_BCR           VIDEO_GPIO_BCR

// ============================================================================
// Internal State Variables
// ============================================================================
volatile u32 line_counter = 0;  // Current line within frame (0..311)

// ============================================================================
// IRQ Handler Declaration
// ============================================================================
// Handler placed in RAM (.data.vector_handler) for faster execution - critical for cycle-accurate pixel timing.
__attribute__((section(".data.vector_handler"))) void TIM2_IRQHandler() __attribute__((interrupt));

// ============================================================================
// Generic TIM2 IRQ Handler
// ============================================================================
// Common interrupt handler for all video modes.
// Calls VMODE-specific rca_render_line() and rca_frame_reset().

void TIM2_IRQHandler() {
    TIM2->INTFR = ~(1 << 2); // Clear CC2 interrupt flag (write 0 to clear)

    u32 line = line_counter;

    if (line < VBLANK_TOP_LINES) {
        line_counter = line + 1;
        VIDEO_GPIO_BCR = VIDEO_PIN_MASK;
        return;
    }

    // FAST PATH: Active area (most common case - 240 of 312 lines)
    u32 active_line = line - VBLANK_TOP_LINES;
    if (active_line < ACTIVE_LINES) {
        line_counter = line + 1;
        DispLine = line;  // Update DispLine in fast path for accurate rca_in_vsync()
        rca_render_line(active_line);
        VIDEO_GPIO_BCR = VIDEO_PIN_MASK;
        return;
    }

    // SLOW PATH: VBLANK and VSYNC (less time-critical)
    if (++line >= TOTAL_LINES) {
        line = 0;
        DispFrame++;
        if (frame_end_callback) frame_end_callback();
    }
    line_counter = line;
    DispLine = line;

    // VSYNC handling
    if (line == VSYNC_START_LINE) {
        TIM2->CH1CVR = TIM_VSYNC_LONG;
    } else if (line == VSYNC_END_LINE) {
        TIM2->CH1CVR = TIM_HSYNC_PULSE;
        rca_frame_reset();
    }

    VIDEO_GPIO_BCR = VIDEO_PIN_MASK;
}

// ============================================================================
// Public API Functions
// ============================================================================

/**
 * Initialize RCA composite video output
 *
 * Configures:
 *   - HSI oscillator trim for ~50 MHz operation
 *   - GPIO: SYNC_PIN (PD4) as TIM2_CH1 PWM output for HSYNC
 *   - GPIO: VIDEO_PIN (PD6) as video data output
 *   - TIM2: PWM mode for HSYNC pulses, CC2 interrupt for pixel rendering
 *   - NVIC: Enable TIM2 interrupt
 */
void rca_init() {
    // Set INTSYSCR to 0x03 (same as ch32libsdk) to fix timing jitter
    asm volatile("li t0, 0x03; csrw 0x804, t0" ::: "t0");

    // Initialize VMODE-specific state variables
    rca_vmode_init();

    // Trim HSI to ~50 MHz (max trim value = 31)
    RCC_HSITrim(31);

    // Enable required clocks
    RCC_PDClkEnable();   // Port D clock (VIDEO_PIN and SYNC_PIN)
    RCC_PCClkEnable();   // Port C clock

    // Configure GPIO pins
    GPIO_Mode(SYNC_PIN, GPIO_MODE_AF_FAST);   // SYNC_PIN (PD4) = TIM2_CH1 PWM output

#if USE_RCA_SPI
    RCC_SPI1ClkEnable();
    RCC_SPI1Reset();
    SPI1_Baud(SPI_BAUD_PRESCALER);
    SPI1_Data8();
    SPI1_MSB();
    SPI1_NSSSw();
    SPI1_NSSHigh();
    SPI1_SSDisable();
    SPI1_Bidi1();
    SPI1_TxBidi();
    SPI1_Master();
    SPI1_Enable();

    GPIO_Remap_SPI1(0);
    GPIO_Mode(VIDEO_PIN, GPIO_MODE_AF_FAST);
#else
    // Bit-bang mode: Configure VIDEO_PIN as GPIO output
    GPIO_Mode(VIDEO_PIN, GPIO_MODE_OUT_FAST); // VIDEO_PIN (PD6) = video data output
#endif

#if USE_RVPC
#if CH32V003
    GPIO_Remap_TIM2(2);
#else
    GPIO_Remap_TIM2(1);
#endif
#endif

    // Configure TIM2 for HSYNC generation
    TIM2_InitPWM(1, 1, TIM_LINE_PERIOD, TIM_HSYNC_PULSE, False);
    TIM2_Comp2(TIM_VIDEO_START);  // CC2 triggers at video start time
    TIM2_OC2Mode(TIM_COMP_FREEZE);
    TIM2_OC2PreEnable();
    TIM2_CC2Enable();
    TIM2_CC2IntClr();
    TIM2_CC2IntEnable();

    // Enable TIM2 interrupt for pixel rendering
    NVIC_IRQEnable(IRQ_TIM2);
}

/**
 * Terminate RCA composite video output
 *
 * Disables TIM2 interrupts and timer, resets GPIO pins to low,
 * restores HSI trim to default value (16).
 */
void rca_stop() {
    // Disable interrupt first to stop video generation
    NVIC_IRQDisable(IRQ_TIM2);
    TIM2_CC2IntDisable();
    TIM2_CC2Disable();
    TIM2_CC2IntClr();

    // Stop and reset TIM2
    TIM2_Disable();
    TIM2_Reset();

#if USE_RCA_SPI
    // Disable SPI
    SPI1_Disable();
#endif

    // Set video output pins to low (blank screen)
    GPIO_PinReset(VIDEO_PIN);
    GPIO_PinReset(SYNC_PIN);

    // Restore HSI trim to default value (~24 MHz)
    RCC_HSITrim(16);
}

void rca_set_frame_callback(void (*callback)()) {
    frame_end_callback = callback;
}

void rca_update_font() {
    rca_vmode_update_font();
}

