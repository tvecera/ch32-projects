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
 * VMODE 1: 160x120 Graphics Mode
 * ============================================================================
 *
 * Resolution: 160x120 pixels, 2400 bytes framebuffer
 * Vertical scale: 2x (240 scanlines)
 * Pixel timing: ~12 cycles/pixel at 50 MHz
 *
 * ============================================================================
 */
#ifndef _RCA_VMODE_1_H
#define _RCA_VMODE_1_H

// VMODE 1 - Graphics mode 160x120 pixels, only for CH32V002
// RAM: framebuffer 2400 B + handler 330 B = 2730 B
#if VMODE == 1

extern u8 FrameBuf[];

#define WIDTH       160                // width in pixels
#define HEIGHT      120                // height in graphics lines (must be even number - due render "next scanline")
#define WIDTHBYTE   (WIDTH/8)          // width in bytes (= 20)
#define FRAMESIZE   (WIDTHBYTE*HEIGHT) // size of frame buffer in bytes (= 20*120 = 2400 bytes)
#define TEXTWIDTH   (WIDTH/8)          // text width in characters (= 20)
#define TEXTHEIGHT  (HEIGHT/8)         // text height in rows (= 15; 1 character = 8x8 pixels)

#ifndef TIM_VIDEO_START
#if USE_RCA_SPI
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(23000ULL)
#else
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(11300ULL)
#endif
#endif

#if USE_RCA_SPI
#define ACTIVE_LINES        120 // 120 source lines, no vertical scale
#define VBLANK_TOP_LINES    96  // (312 - 120) / 2 = center vertically
#define VSCALE_SHIFT        0   // no vertical scaling
#define SPI_BAUD_PRESCALER  SPI_BAUD_DIV8
#else
// Bit-bang mode: 2x vertical scale
#define ACTIVE_LINES        240 // 120 source lines * 2 vertical scale
#define VBLANK_TOP_LINES    37  // shift the image down for proper centering
#define VSCALE_SHIFT        1   // >> 1 = divide by 2
#endif

// Fast send of 8 pixels on PD6 for graphics mode 160x120
// Each pixel takes ~12 cycles at 50 MHz = 0.24 us/pixel
// Faster than VMODE 0 to fit 160 pixels per line
// PD6 = bit 6 in GPIOD, so we shift data bits to position 6
//
// Cycle timing per instruction:
//   andi/slli/srli: 1 cycle
//   sw (store):     1 cycle
//   c.li:           1 cycle
//   c.addi:         1 cycle
//   c.bnez taken:   2 cycles, not taken: 1 cycle
//
// Loop timing for counter=3: 2*(1+2) + (1+1) = 8 cycles
INLINE static void SendByteBitbangFast(u8 data) {
    register u32 gpiod_data = GPIOD_BASE + 0x0C; // GPIOD OUTDR offset

    asm volatile(
        // Bit 7 (MSB): andi[1] + srli[1] + sw[1] + li[1] + loop[8] = 12 cycles
        "andi   a2, %0, 0x80        \n" // [1] mask bit 7
        "c.srli a2, 1               \n" // [1] shift right 1: 0x80 -> 0x40
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 3               \n" // [1] loop counter
        "1: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 1b              \n" // [2/1] branch: 3+3+2 = 8 cycles

        // Bit 6: andi[1] + sw[1] + li[1] + loop[8] = 11 cycles (no shift)
        "andi   a2, %0, 0x40        \n" // [1] mask bit 6
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 3               \n" // [1] loop counter
        "2: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 2b              \n" // [2/1] branch: 8 cycles

        // Bit 5: andi[1] + slli[1] + sw[1] + li[1] + loop[8] = 12 cycles
        "andi   a2, %0, 0x20        \n" // [1] mask bit 5
        "c.slli a2, 1               \n" // [1] shift left 1: 0x20 -> 0x40
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 3               \n" // [1] loop counter
        "3: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 3b              \n" // [2/1] branch: 8 cycles

        // Bit 4: andi[1] + slli[1] + sw[1] + li[1] + loop[8] = 12 cycles
        "andi   a2, %0, 0x10        \n" // [1] mask bit 4
        "c.slli a2, 2               \n" // [1] shift left 2: 0x10 -> 0x40
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 3               \n" // [1] loop counter
        "4: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 4b              \n" // [2/1] branch: 8 cycles

        // Bit 3: andi[1] + slli[1] + sw[1] + li[1] + loop[8] = 12 cycles
        "andi   a2, %0, 0x08        \n" // [1] mask bit 3
        "c.slli a2, 3               \n" // [1] shift left 3: 0x08 -> 0x40
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 3               \n" // [1] loop counter
        "5: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 5b              \n" // [2/1] branch: 8 cycles

        // Bit 2: andi[1] + slli[1] + sw[1] + li[1] + loop[8] = 12 cycles
        "andi   a2, %0, 0x04        \n" // [1] mask bit 2
        "c.slli a2, 4               \n" // [1] shift left 4: 0x04 -> 0x40
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 3               \n" // [1] loop counter
        "6: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 6b              \n" // [2/1] branch: 8 cycles

        // Bit 1: andi[1] + slli[1] + sw[1] + li[1] + loop[8] = 12 cycles
        "andi   a2, %0, 0x02        \n" // [1] mask bit 1
        "c.slli a2, 5               \n" // [1] shift left 5: 0x02 -> 0x40
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 3               \n" // [1] loop counter
        "7: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 7b              \n" // [2/1] branch: 8 cycles

        // Bit 0 (LSB): andi[1] + slli[1] + sw[1] + nop[3] = 6 cycles
        // Shorter delay for last pixel (next byte setup overlaps)
        "andi   a2, %0, 0x01        \n" // [1] mask bit 0
        "c.slli a2, 6               \n" // [1] shift left 6: 0x01 -> 0x40
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.nop                      \n" // [1] delay
        "c.nop                      \n" // [1] delay
        :
        : "r" (data), "r" (gpiod_data)
        : "a2", "a3", "memory"
    );
}

// Render one active scanline (called from generic IRQ handler)
INLINE static void rca_render_line(u32 active_line) {
    u32 y = active_line >> VSCALE_SHIFT; // 0..119 (source line)
    const u8* ptr = &FrameBuf[y * WIDTHBYTE];

#if USE_RCA_SPI
    const u8* end = ptr + WIDTHBYTE;
    // Force 4-byte alignment for loop to ensure consistent timing (misaligned branch targets can cause fetch jitter)
    asm volatile(".align 2");
    do {
        while ((SPI1->STATR & (1<<1)) == 0) {}
        SPI1->DATAR = *ptr++;
    } while (ptr < end);
    while ((SPI1->STATR & (1<<1)) == 0) {}
    SPI1->DATAR = 0x00;
#else
    for (u8 i = 0; i < WIDTHBYTE; i++) {
        SendByteBitbangFast(*ptr++);
    }
#endif
}

// Reset state at frame end (nothing for graphics mode)
INLINE static void rca_frame_reset() {
    // No state to reset for graphics mode
}

INLINE static void rca_vmode_init() {
    // Nothing specific to init for this graphics mode
}

INLINE static void rca_vmode_update_font() {
    // Nothing specific to update for this graphics mode
}

#endif // VMODE == 1
#endif // _RCA_VMODE_1_H
