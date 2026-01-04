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
// RAM: framebuffer 414 B + LUT 56 B + LUT 256 B + state 8 B + handler ~380 B = ~1114 B (SPI)
// Flash: font 8x8 pixels 2048/4096 B (64/128 chars)
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
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(22300ULL)
#else
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(20300ULL)
#endif
#else
#if CH32V003
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(11500ULL)
#else
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(10600ULL)
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

#if USE_RCA_SPI
// Lookup table: byte → 2 bits (high nibble → bit1, low nibble → bit0)
static const u8 byte_to_2bits[256] = {
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x0X: low nibble 0→0, 1-F→1
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0x1X: high nibble 1→1
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0x2X
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0x3X
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0x4X
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0x5X
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0x6X
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0x7X
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0x8X
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0x9X
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0xAX
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0xBX
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0xCX
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0xDX
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0xEX
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 // 0xFX
};

// Convert nibble-format font data (4 bytes) to SPI byte (8 bits)
// Uses LUT for constant-time conversion (4 lookups, no branches)
INLINE static u8 pack_nibbles_to_spi(const u8* src) {
    return (byte_to_2bits[src[0]] << 6) |
        (byte_to_2bits[src[1]] << 4) |
        (byte_to_2bits[src[2]] << 2) |
        byte_to_2bits[src[3]];
}
#endif

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
    u8 ch;
    const u8* text_row = &FrameBuf[current_char_row * WIDTHBYTE];
    const u8* font_row = font_row_ptr_lut[current_line_in_row];

#if USE_RCA_SPI
    u8 spi_byte;
    // Force 4-byte alignment for loop to ensure consistent timing (misaligned branch targets can cause fetch jitter)
    asm volatile(".align 2");
    for (ch = 0; ch < WIDTHBYTE; ch++) {
        const u8* char_glyph = &font_row[text_row[ch] << 2];
        spi_byte = pack_nibbles_to_spi(char_glyph);
        SPI1->DATAR = spi_byte;
    }
    while (!(SPI1->STATR & (1 << 1))) {}
    SPI1->DATAR = 0x00;
#else
    for (ch = 0; ch < WIDTHBYTE; ch++) {
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
    const u16 stride = DrawFont->char_count * 4;
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
    const u16 stride = DrawFont->char_count * 4;
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
