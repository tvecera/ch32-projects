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
 * VMODE 7: 32x24 Text Mode
 * ============================================================================
 *
 * Resolution: 32x24 characters (256x192 effective pixels)
 * Font: 8x8 pixels per character with wider pixels
 * Framebuffer: 768 bytes (ASCII characters)
 *
 * ============================================================================
 */

#ifndef _RCA_VMODE_7_H
#define _RCA_VMODE_7_H

// VMODE 7 - Text mode 32x24 characters of 8x8 pixels with custom fonts of 64/128 characters
// RAM: framebuffer 768 B + state 20 B + handler ~430 B = ~1218 B (bit-bang)
// RAM: framebuffer 768 B + LUT 256 B + state 20 B + handler ~400 B = ~1444 B (SPI)
// Flash: font 8x8 pixels 2048/4096 B (64/128 chars)
#if VMODE == 7

#include "../fonts/font_desc.h"
extern const font_desc_t* DrawFont;
extern u8 FrameBuf[];

#define WIDTH       32                 // width in characters
#define HEIGHT      24                 // height in text rows
#define WIDTHBYTE   WIDTH              // width in bytes (= 32)
#define FRAMESIZE   (WIDTHBYTE*HEIGHT) // size of frame buffer in bytes (= 32*24 = 768 bytes)
#define TEXTWIDTH   WIDTH              // text width in characters (= 32)
#define TEXTHEIGHT  HEIGHT             // text height in rows (= 24)

#ifndef TIM_VIDEO_START
#if USE_RCA_SPI
#if CH32V003
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(18600ULL)
#else
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(16100ULL)
#endif
#else
#if CH32V003
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(12900ULL)
#else
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(11700ULL)
#endif
#endif
#endif

#define ACTIVE_LINES        192  // 24 text rows * 8 scanlines per row
#define VBLANK_TOP_LINES    60   // shift the image down for proper centering in safe area
#define LINES_PER_CHAR      8    // scanlines per character row
#define SPI_BAUD_PRESCALER  SPI_BAUD_DIV8

// State variables for text mode rendering
static u32 current_line_in_row = 0;     // Current scanline within text row (0..7)
static const u8* current_font_row = 0;  // Pointer to current font row data
static const u8* current_text_row = 0;  // Pointer to current text row in FrameBuf

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
    2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3  // 0xFX
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

// Send 8 pixels from font character on PD6 for text mode 32x24
// Font format: 4 bytes per character (2 pixels per byte in nibbles)
// Wider pixels with NOP delays for horizontal stretching
// Each character = 8 pixels, total ~56 cycles/char = 7 cycles/pixel
//
// Cycle timing per instruction:
//   lbu (load byte):   1 cycle (from RAM)
//   srli/slli/andi:    1 cycle
//   sw (store):        1 cycle
//   nop:               1 cycle (c.nop = 2 bytes, 1 cycle each)
//
// Pixel timing: lbu[1] + srli[1] + slli[1] + sw[1] + 2*nop[2] = 6 cycles (first)
//               andi[1] + slli[1] + sw[1] + 2*nop[2] = 5 cycles (second)
INLINE static void SendCharBitBangTextMode(const u8* font_data) {
    register u32 gpiod_outdr = GPIOD_BASE + 0x0C;

    asm volatile(
        // Byte 0: pixels 0-1 (11 cycles)
        "lbu    t0, 0(%0)               \n" // [1] load font byte from RAM
        "srli   a2, t0, 4               \n" // [1] extract high nibble (pixel 0)
        "slli   a2, a2, 4               \n" // [1] shift to bit 4-7 for GPIO
        "sw     a2, 0(%1)               \n" // [1] output pixel 0
        "nop; nop;                      \n" // [2] pixel width delay

        "andi   a2, t0, 0x0f            \n" // [1] extract low nibble (pixel 1)
        "slli   a2, a2, 4               \n" // [1] shift to bit 4-7
        "sw     a2, 0(%1)               \n" // [1] output pixel 1
        "nop; nop;                      \n" // [2] pixel width delay

        // Byte 1: pixels 2-3 (11 cycles)
        "lbu    t0, 1(%0)               \n" // [1]
        "srli   a2, t0, 4               \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]
        "nop; nop;                      \n" // [2]

        "andi   a2, t0, 0x0f            \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]
        "nop; nop;                      \n" // [2]

        // Byte 2: pixels 4-5 (11 cycles)
        "lbu    t0, 2(%0)               \n" // [1]
        "srli   a2, t0, 4               \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]
        "nop; nop;                      \n" // [2]

        "andi   a2, t0, 0x0f            \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]
        "nop; nop;                      \n" // [2]

        // Byte 3: pixels 6-7 (9 cycles, no delay on last pixel)
        "lbu    t0, 3(%0)               \n" // [1]
        "srli   a2, t0, 4               \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]
        "nop; nop;                      \n" // [2]

        "andi   a2, t0, 0x0f            \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]
        :
        : "r" (font_data), "r" (gpiod_outdr)
        : "a2", "a3", "t0", "memory"
    );
}

// Render one active scanline (32 chars, called from generic IRQ handler)
// FrameBuf contains font indices (mapped by print.c), use directly
INLINE static void rca_render_line(u32 active_line) {
    (void)active_line; // unused, we track state internally

    const u8* text_row = current_text_row;
    const u8* font_row = current_font_row;

#if USE_RCA_SPI
    u8 spi_byte;
    // Force 4-byte alignment for loop to ensure consistent timing (misaligned branch targets can cause fetch jitter)
    asm volatile(".align 2");
    for (u8 ch = 0; ch < WIDTHBYTE; ch++) {
        const u8* char_glyph = &font_row[text_row[ch] << 2];
        spi_byte = pack_nibbles_to_spi(char_glyph);
        SPI1->DATAR = spi_byte;
    }
    while (!(SPI1->STATR & (1 << 1))) {}
    SPI1->DATAR = 0x00;
#else
    for (u8 ch = 0; ch < WIDTHBYTE; ch++) {
        const u8* char_glyph = &font_row[text_row[ch] << 2];
        SendCharBitBangTextMode(char_glyph);
    }
#endif

    // Update counters for next scanline
    current_line_in_row++;
    current_font_row += DrawFont->char_count * 4;
    if (current_line_in_row >= LINES_PER_CHAR) {
        current_line_in_row = 0;
        current_font_row = DrawFont->data;
        current_text_row += WIDTHBYTE;
    }
}

// Reset state at frame end
INLINE static void rca_frame_reset() {
    current_line_in_row = 0;
    current_font_row = DrawFont->data;
    current_text_row = FrameBuf;
}

// Initialize text mode state
INLINE static void rca_vmode_init() {
    current_line_in_row = 0;
    current_font_row = DrawFont->data;
    current_text_row = FrameBuf;
}

// Update font pointers after font change
INLINE static void rca_vmode_update_font() {
    current_line_in_row = 0;
    current_font_row = DrawFont->data;
    current_text_row = FrameBuf;
}

#endif // VMODE == 7
#endif // _RCA_VMODE_7_H
