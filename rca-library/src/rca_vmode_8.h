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
 * VMODE 8: 23x18 Text Mode
 * ============================================================================
 *
 * Resolution: 23x18 characters (184x252 effective pixels)
 * Font: 8x14 pixels per character (8 font lines stretched to 14 scanlines)
 * Framebuffer: 414 bytes (ASCII characters)
 *
 * ============================================================================
 */
#ifndef _RCA_VMODE_8_H
#define _RCA_VMODE_8_H

// VMODE 8 - Text mode 23x18 characters of 8x8 pixels (14 scanlines/row) with custom fonts of 64/128 characters
// RAM: framebuffer 414 B + LUT 56 B + state 8 B + handler ~418 B = ~896 B (bit-bang)
// RAM: framebuffer 414 B + LUT 56 B + state 8 B + handler ~350 B = ~828 B (SPI)
// Flash: font 8x8 pixels 2048 B (SPI) or 4096 B nibble-packed (bit-bang)
// Original Olimex RVPC video mode
#if VMODE == 8

#include "../fonts/font_desc.h"
extern const font_desc_t* DrawFont;
extern u8 FrameBuf[];

#define WIDTH       23                 // width in characters
#define HEIGHT      18                 // height in text rows
#define WIDTHBYTE   WIDTH              // width in bytes (= 23)
#define FRAMESIZE   (WIDTHBYTE*HEIGHT) // size of frame buffer in bytes (= 23*18 = 414 bytes)
#define TEXTWIDTH   WIDTH              // text width in characters (= 23)
#define TEXTHEIGHT  HEIGHT             // text height in rows (= 18; 1 character = 8x8 pixels)

#ifndef TIM_VIDEO_START
#if USE_RCA_SPI
#if CH32V003
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(19800ULL)
#else
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(19800ULL)
#endif
#else
#if CH32V003
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(11600ULL)
#else
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(11000ULL)
#endif
#endif
#endif

#define ACTIVE_LINES        252  // 18 text rows * 16 scanlines per row
#define VBLANK_TOP_LINES    30   // center vertically
#define LINES_PER_CHAR      14   // scanlines per character row (stretched from 8)
#define SPI_BAUD_PRESCALER  SPI_BAUD_DIV8

// State variables for text mode rendering
static u32 current_char_row = 0;    // Current text row (0..17)
static u32 current_line_in_row = 0; // Current scanline within row (0..15)

// Font row pointer lookup table for 14 scanlines per character row
// Pattern: 0,0,1,1,2,3,3,4,5,5,6,6,7,7 (stretches 8 font lines to 14 scanlines)
static const u8* font_row_ptr_lut[14];

// Send 8 pixels from font character on PD6 for text mode 23x18
// Font format: 4 bytes per character (2 pixels per byte in nibbles)
// Each character = 8 pixels with delay loops for wider pixels
//
// Cycle timing per instruction:
//   lbu (load byte):   1 cycle (from RAM via LUT pointer)
//   srli/slli/andi:    1 cycle
//   sw (store):        1 cycle
//   c.li:              1 cycle
//   c.addi:            1 cycle
//   c.bnez taken:      2 cycles, not taken: 1 cycle
//
// Loop timing for counter=2: 1*(1+2) + (1+1) = 5 cycles
// Pixel timing: lbu[1] + srli[1] + slli[1] + sw[1] + li[1] + loop[5] = 10 cycles (first)
//               andi[1] + slli[1] + sw[1] + li[1] + loop[5] = 9 cycles (second)
// Total per char: 4*(10+9) - last_delay = ~72 cycles
INLINE static void SendCharBitBangTextMode(const u8* font_data) {
    register u32 gpiod_outdr = GPIOD_BASE + 0x0C;

    asm volatile(
        // Byte 0: pixels 0-1
        "lbu    t0, 0(%0)               \n" // [1] load font byte from RAM
        "srli   a2, t0, 4               \n" // [1] extract high nibble (pixel 0)
        "slli   a2, a2, 4               \n" // [1] shift to bit 4-7 for GPIO
        "sw     a2, 0(%1)               \n" // [1] output pixel 0
        "c.li   a3, 2                   \n" // [1] loop counter
        "1: c.addi a3, -1               \n" // [1] decrement
        "c.bnez a3, 1b                  \n" // [2/1] branch: 3+2 = 5 cycles

        "andi   a2, t0, 0x0f            \n" // [1] extract low nibble (pixel 1)
        "slli   a2, a2, 4               \n" // [1] shift to bit 4-7
        "sw     a2, 0(%1)               \n" // [1] output pixel 1
        "c.li   a3, 2                   \n" // [1] loop counter
        "2: c.addi a3, -1               \n" // [1] decrement
        "c.bnez a3, 2b                  \n" // [2/1] branch: 5 cycles

        // Byte 1: pixels 2-3
        "lbu    t0, 1(%0)               \n" // [1]
        "srli   a2, t0, 4               \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]
        "c.li   a3, 2                   \n" // [1]
        "3: c.addi a3, -1               \n" // [1]
        "c.bnez a3, 3b                  \n" // [2/1]

        "andi   a2, t0, 0x0f            \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]
        "c.li   a3, 2                   \n" // [1]
        "4: c.addi a3, -1               \n" // [1]
        "c.bnez a3, 4b                  \n" // [2/1]

        // Byte 2: pixels 4-5
        "lbu    t0, 2(%0)               \n" // [1]
        "srli   a2, t0, 4               \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]
        "c.li   a3, 2                   \n" // [1]
        "5: c.addi a3, -1               \n" // [1]
        "c.bnez a3, 5b                  \n" // [2/1]

        "andi   a2, t0, 0x0f            \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]
        "c.li   a3, 2                   \n" // [1]
        "6: c.addi a3, -1               \n" // [1]
        "c.bnez a3, 6b                  \n" // [2/1]

        // Byte 3: pixels 6-7 (no delay on last pixel)
        "lbu    t0, 3(%0)               \n" // [1]
        "srli   a2, t0, 4               \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]

        "andi   a2, t0, 0x0f            \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]
        :
        : "r" (font_data), "r" (gpiod_outdr)
        : "a2", "a3", "t0", "memory"
    );
}

// Render one active scanline (23 chars, called from generic IRQ handler)
// FrameBuf contains font indices (mapped by print.c), use directly
INLINE static void rca_render_line(u32 active_line) {
    (void)active_line; // unused, we track state internally
    const u8* text_row = &FrameBuf[current_char_row * WIDTHBYTE];
    const u8* font_row = font_row_ptr_lut[current_line_in_row];

#if USE_RCA_SPI
    // SPI mode: graphics font format (1 byte = 8 pixels, direct to SPI)
    // Font layout: font[line * 256 + char_index], LUT provides line pointer
    // ASM loop based on babypc_vga_asm.S pattern
    //
    // SPI DIV8 @ 50MHz = 6.25 MHz SPI clock = 128 CPU cycles per byte
    // Loop body must take 128 cycles to match SPI timing
    asm volatile(
        "   li      a3, %[width]            \n" // loop counter = 23
        ".align 2,,                         \n" // align loop start
        "2:                                 \n" // main loop: 128 cycles per iteration
        "   lbu     a0, 0(%[text])          \n" // [2] load char index from FrameBuf
        "   addi    %[text], %[text], 1     \n" // [1] advance text pointer
        "   add     a0, %[font], a0         \n" // [1] compute font address
        "   lbu     a0, 0(a0)               \n" // [3] load font byte from Flash
        "   addi    a3, a3, -1              \n" // [1] decrement loop counter
        "   sw      a0, 0x0C(%[spi])        \n" // [2] write to SPI DATAR
        // delay loop: 28 iterations * 4 cycles = 112 cycles
        "   li      a2, 12                  \n" // [1] delay counter
        ".align 2,,                         \n"
        "1: addi    a2, a2, -1              \n" // [1]
        "   bnez    a2, 1b                  \n" // [3 taken, 1 not taken]
        "   nop                             \n" // [1] padding
        "   nop                             \n" // [1] padding
        "   bnez    a3, 2b                  \n" // [3 taken, 1 not taken]
        "   sw      zero, 0x0C(%[spi])      \n" // send trailing black
        :
        : [text] "r" (text_row),
          [font] "r" (font_row),
          [spi]  "r" (SPI1_BASE),
          [width] "i" (WIDTHBYTE)
        : "a0", "a2", "a3", "memory"
    );
#else
    // Bitbang mode: nibble-packed font format (4 bytes per char)
    for (u8 ch = 0; ch < WIDTHBYTE; ch++) {
        const u8* char_glyph = &font_row[text_row[ch] << 2];
        SendCharBitBangTextMode(char_glyph);
    }
#endif

    // Update counters for next scanline
    current_line_in_row++;
    if (current_line_in_row >= LINES_PER_CHAR) {
        current_line_in_row = 0;
        current_char_row++;
    }
}

// Reset state at frame end
INLINE static void rca_frame_reset() {
    current_char_row = 0;
    current_line_in_row = 0;
}

// Initialize text mode state and font row LUT
INLINE static void rca_vmode_init() {
    current_char_row = 0;
    current_line_in_row = 0;
    // Initialize font row pointer LUT
    // Pattern: 0,0,1,1,2,3,3,4,5,5,6,6,7,7 (stretches 8 font lines to 14 scanlines)
#if USE_RCA_SPI
    const u16 stride = 256; // SPI font: 256 bytes per line (font[line * 256 + char])
#else
    const u16 stride = DrawFont->char_count * 4; // Nibble font: char_count * 4 bytes per line
#endif
    const u8* fdata = DrawFont->data;
    font_row_ptr_lut[0] = &fdata[0 * stride];
    font_row_ptr_lut[1] = &fdata[0 * stride];
    font_row_ptr_lut[2] = &fdata[1 * stride];
    font_row_ptr_lut[3] = &fdata[1 * stride];
    font_row_ptr_lut[4] = &fdata[2 * stride];
    font_row_ptr_lut[5] = &fdata[3 * stride];
    font_row_ptr_lut[6] = &fdata[3 * stride];
    font_row_ptr_lut[7] = &fdata[4 * stride];
    font_row_ptr_lut[8] = &fdata[5 * stride];
    font_row_ptr_lut[9] = &fdata[5 * stride];
    font_row_ptr_lut[10] = &fdata[6 * stride];
    font_row_ptr_lut[11] = &fdata[6 * stride];
    font_row_ptr_lut[12] = &fdata[7 * stride];
    font_row_ptr_lut[13] = &fdata[7 * stride];
}

// Update font pointers after font change
INLINE static void rca_vmode_update_font() {
    current_char_row = 0;
    current_line_in_row = 0;
#if USE_RCA_SPI
    const u16 stride = 256; // SPI font: 256 bytes per line (font[line * 256 + char])
#else
    const u16 stride = DrawFont->char_count * 4; // Nibble font: char_count * 4 bytes per line
#endif
    const u8* fdata = DrawFont->data;
    font_row_ptr_lut[0] = &fdata[0 * stride];
    font_row_ptr_lut[1] = &fdata[0 * stride];
    font_row_ptr_lut[2] = &fdata[1 * stride];
    font_row_ptr_lut[3] = &fdata[1 * stride];
    font_row_ptr_lut[4] = &fdata[2 * stride];
    font_row_ptr_lut[5] = &fdata[3 * stride];
    font_row_ptr_lut[6] = &fdata[3 * stride];
    font_row_ptr_lut[7] = &fdata[4 * stride];
    font_row_ptr_lut[8] = &fdata[5 * stride];
    font_row_ptr_lut[9] = &fdata[5 * stride];
    font_row_ptr_lut[10] = &fdata[6 * stride];
    font_row_ptr_lut[11] = &fdata[6 * stride];
    font_row_ptr_lut[12] = &fdata[7 * stride];
    font_row_ptr_lut[13] = &fdata[7 * stride];
}

#endif // VMODE == 8
#endif // _RCA_VMODE_8_H
