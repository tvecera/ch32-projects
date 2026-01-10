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
 * VMODE 0: 128x64 Graphics Mode
 * ============================================================================
 *
 * Resolution: 128x64 pixels, 1024 bytes framebuffer
 * Vertical scale: 4x (256 scanlines)
 * Pixel timing: ~14 cycles/pixel at 50 MHz
 *
 * ============================================================================
 */
#ifndef _RCA_VMODE_0_H
#define _RCA_VMODE_0_H

// VMODE 0 - Graphics mode 128x64 pixels
// RAM: framebuffer 1024 B + handler 306 B = 1330 B
#if VMODE == 0

extern u8 FrameBuf[];

#define WIDTH       128         // width in pixels
#define HEIGHT      64          // height in graphics lines (must be even number - due render "next scanline")
#define WIDTHBYTE   (WIDTH/8)   // width in bytes (= 16)
#define FRAMESIZE   (WIDTHBYTE*HEIGHT) // size of frame buffer in bytes (= 16*64 = 1024 bytes)
#define TEXTWIDTH   (WIDTH/8)   // text width in characters (= 16)
#define TEXTHEIGHT  (HEIGHT/8)  // text height in rows (= 8; 1 character = 8x8 pixels)

#ifndef TIM_VIDEO_START
#if USE_RCA_SPI
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(14400ULL)
#else
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(10800ULL)
#endif
#endif

#define ACTIVE_LINES          256  // 64 source lines * 4 vertical scale
#define VBLANK_TOP_LINES      29   // shift the image down
#define VSCALE_SHIFT          2    // >> 2 = divide by 4
#define SPI_BAUD_PRESCALER    SPI_BAUD_DIV16

// Fast send of 8 pixels for graphics mode 128x64
// Each pixel takes ~14 cycles at 50 MHz = 0.28 us/pixel
//
// Pin configuration:
//   USE_TINYBOY: PC2 = bit 2 in GPIOC, shift data bits to position 2
//   Default:     PD6 = bit 6 in GPIOD, shift data bits to position 6
//
// Cycle timing per instruction:
//   andi/slli/srli: 1 cycle
//   sw (store):     1 cycle
//   c.li:           1 cycle
//   c.addi:         1 cycle
//   c.bnez taken:   2 cycles, not taken: 1 cycle
//
// Loop timing for counter=4: 3*(1+2) + (1+1) = 11 cycles

#if USE_TINYBOY
#define BB_GPIO_BASE    GPIOC_BASE
#define BB_SHIFT_BIT7   "c.srli a2, 5 \n"
#define BB_SHIFT_BIT6   "c.srli a2, 4 \n"
#define BB_SHIFT_BIT5   "c.srli a2, 3 \n"
#define BB_SHIFT_BIT4   "c.srli a2, 2 \n"
#define BB_SHIFT_BIT3   "c.srli a2, 1 \n"
#define BB_SHIFT_BIT2   ""
#define BB_SHIFT_BIT1   "c.slli a2, 1 \n"
#define BB_SHIFT_BIT0   "c.slli a2, 2 \n"
#else
#define BB_GPIO_BASE    GPIOD_BASE
#define BB_SHIFT_BIT7   "c.srli a2, 1 \n"
#define BB_SHIFT_BIT6   ""
#define BB_SHIFT_BIT5   "c.slli a2, 1 \n"
#define BB_SHIFT_BIT4   "c.slli a2, 2 \n"
#define BB_SHIFT_BIT3   "c.slli a2, 3 \n"
#define BB_SHIFT_BIT2   "c.slli a2, 4 \n"
#define BB_SHIFT_BIT1   "c.slli a2, 5 \n"
#define BB_SHIFT_BIT0   "c.slli a2, 6 \n"
#endif

INLINE static void SendByteBitBang(u8 data) {
    register u32 gpio_data = BB_GPIO_BASE + 0x0C;

    asm volatile(
        // Bit 7 (MSB): andi[1] + srli[1] + sw[1] + li[1] + loop[11] = 15 cycles
        "andi   a2, %0, 0x80        \n" // [1] mask bit 7
        BB_SHIFT_BIT7
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 4               \n" // [1] loop counter
        "1: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 1b              \n" // [2/1] branch: 3+3+3+2 = 11 cycles

        // Bit 6: andi[1] + sw[1] + li[1] + loop[11] = 14 cycles (no shift needed)
        "andi   a2, %0, 0x40        \n" // [1] mask bit 6
        BB_SHIFT_BIT6
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 4               \n" // [1] loop counter
        "2: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 2b              \n" // [2/1] branch: 11 cycles

        // Bit 5: andi[1] + slli[1] + sw[1] + li[1] + loop[11] = 15 cycles
        "andi   a2, %0, 0x20        \n" // [1] mask bit 5
        BB_SHIFT_BIT5
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 4               \n" // [1] loop counter
        "3: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 3b              \n" // [2/1] branch: 11 cycles

        // Bit 4: andi[1] + slli[1] + sw[1] + li[1] + loop[11] = 15 cycles
        "andi   a2, %0, 0x10        \n" // [1] mask bit 4
        BB_SHIFT_BIT4
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 4               \n" // [1] loop counter
        "4: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 4b              \n" // [2/1] branch: 11 cycles

        // Bit 3: andi[1] + slli[1] + sw[1] + li[1] + loop[11] = 15 cycles
        "andi   a2, %0, 0x08        \n" // [1] mask bit 3
        BB_SHIFT_BIT3
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 4               \n" // [1] loop counter
        "5: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 5b              \n" // [2/1] branch: 11 cycles

        // Bit 2: andi[1] + slli[1] + sw[1] + li[1] + loop[11] = 15 cycles
        "andi   a2, %0, 0x04        \n" // [1] mask bit 2
        BB_SHIFT_BIT2
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 4               \n" // [1] loop counter
        "6: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 6b              \n" // [2/1] branch: 11 cycles

        // Bit 1: andi[1] + slli[1] + sw[1] + li[1] + loop[11] = 15 cycles
        "andi   a2, %0, 0x02        \n" // [1] mask bit 1
        BB_SHIFT_BIT1
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 4               \n" // [1] loop counter
        "7: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 7b              \n" // [2/1] branch: 11 cycles

        // Bit 0 (LSB): andi[1] + slli[1] + sw[1] + li[1] + loop[5] = 9 cycles
        // Shorter delay for last pixel (next byte setup overlaps)
        "andi   a2, %0, 0x01        \n" // [1] mask bit 0
        BB_SHIFT_BIT0
        "sw     a2, 0(%1)           \n" // [1] output to GPIO
        "c.li   a3, 2               \n" // [1] loop counter
        "8: c.addi a3, -1           \n" // [1] decrement
        "c.bnez a3, 8b              \n" // [2/1] branch: 3+2 = 5 cycles
        :
        : "r" (data), "r" (gpio_data)
        : "a2", "a3", "memory"
    );
}

// Render one active scanline (called from generic IRQ handler)
INLINE static void rca_render_line(u32 active_line) {
    u32 y = active_line >> VSCALE_SHIFT; // 0..63 (source line)
    const u8* ptr = &FrameBuf[y * WIDTHBYTE];

#if USE_RCA_SPI
    const u8* end = ptr + WIDTHBYTE;
    // Force 4-byte alignment for loop to ensure consistent timing (misaligned branch targets can cause fetch jitter)
    asm volatile(".align 2");
    do {
        while ((SPI1->STATR & B1) == 0) {}
        SPI1->DATAR = *ptr++;
    }
    while (ptr < end);
    while ((SPI1->STATR & B1) == 0) {}
    SPI1->DATAR = 0x00;
#else
    for (u8 i = 0; i < WIDTHBYTE; i++) {
        SendByteBitBang(*ptr++);
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

#endif // VMODE == 0
#endif // _RCA_VMODE_0_H
