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
 * VMODE 6: 40x30 Text Mode
 * ============================================================================
 *
 * Resolution: 40x30 characters (320x240 effective pixels)
 * Font: 8x8 pixels per character
 * Framebuffer: 1200 bytes (ASCII characters)
 *
 * ============================================================================
 */

#ifndef _RCA_VMODE_6_H
#define _RCA_VMODE_6_H

// VMODE 6 - Text mode 40x30 characters of 8x8 pixels with custom fonts of 64/128 characters
// RAM: framebuffer 1200 B + state 20 B + handler ~376 B = ~1596 B (bit-bang)
// RAM: framebuffer 1200 B + state 20 B + handler ~350 B = ~1570 B (SPI)
// Flash: font 8x8 pixels 2048 B (SPI) or 4096 B nibble-packed (bit-bang)
#if VMODE == 6

#include "../fonts/font_desc.h"
extern const font_desc_t* DrawFont;
extern u8 FrameBuf[];

#define WIDTH       40                 // width in characters
#define HEIGHT      30                 // height in text rows
#define WIDTHBYTE   WIDTH              // width in bytes (= 40)
#define FRAMESIZE   (WIDTHBYTE*HEIGHT) // size of frame buffer in bytes (= 40*30 = 1200 bytes)
#define TEXTWIDTH   WIDTH              // text width in characters (= 40)
#define TEXTHEIGHT  HEIGHT             // text height in rows (= 30)

#ifndef TIM_VIDEO_START
#if USE_RCA_SPI
#if CH32V003
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(22100ULL)
#else
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(21600ULL)
#endif
#else
#if CH32V003
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(13600ULL)
#else
#define TIM_VIDEO_START     TIM_TICKS_FROM_NS(11700ULL)
#endif
#endif
#endif

#define ACTIVE_LINES        240  // 30 text rows * 8 scanlines per row
#define VBLANK_TOP_LINES    36   // shift the image down for proper centering in safe area
#define LINES_PER_CHAR      8    // scanlines per character row
#define SPI_BAUD_PRESCALER  SPI_BAUD_DIV4

// State variables for text mode rendering
static u32 current_line_in_row = 0;     // Current scanline within text row (0..7)
static const u8* current_font_row = 0;  // Pointer to current font row data
static const u8* current_text_row = 0;  // Pointer to current text row in FrameBuf


// Send 8 pixels from font character on PD6 for text mode 40x30
// Font format: 4 bytes per character (2 pixels per byte in nibbles)
// Each character = 8 pixels, total ~48 cycles/char = 6 cycles/pixel
//
// Cycle timing per instruction:
//   lbu (load byte):   1 cycle (from RAM), 2+ cycles (from Flash with wait states)
//   srli/slli/andi:    1 cycle
//   sw (store):        1 cycle
//
// Pixel timing: lbu[1] + srli[1] + slli[1] + sw[1] = 4 cycles (first pixel of pair)
//               andi[1] + slli[1] + sw[1] = 3 cycles (second pixel of pair)
// Total per char: 4*(4+3) + overhead = ~48 cycles
INLINE static void SendCharBitBangTextMode(const u8* font_data) {
    register u32 gpiod_outdr = GPIOD_BASE + 0x0C;

    asm volatile(
        // Byte 0: pixels 0-1 (7 cycles)
        "lbu    t0, 0(%0)               \n" // [1] load font byte from RAM
        "srli   a2, t0, 4               \n" // [1] extract high nibble (pixel 0)
        "slli   a2, a2, 4               \n" // [1] shift to bit 4-7 for GPIO
        "sw     a2, 0(%1)               \n" // [1] output pixel 0

        "andi   a2, t0, 0x0f            \n" // [1] extract low nibble (pixel 1)
        "slli   a2, a2, 4               \n" // [1] shift to bit 4-7
        "sw     a2, 0(%1)               \n" // [1] output pixel 1

        // Byte 1: pixels 2-3 (7 cycles)
        "lbu    t0, 1(%0)               \n" // [1]
        "srli   a2, t0, 4               \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]

        "andi   a2, t0, 0x0f            \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]

        // Byte 2: pixels 4-5 (7 cycles)
        "lbu    t0, 2(%0)               \n" // [1]
        "srli   a2, t0, 4               \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]

        "andi   a2, t0, 0x0f            \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]

        // Byte 3: pixels 6-7 (7 cycles)
        "lbu    t0, 3(%0)               \n" // [1]
        "srli   a2, t0, 4               \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]

        "andi   a2, t0, 0x0f            \n" // [1]
        "slli   a2, a2, 4               \n" // [1]
        "sw     a2, 0(%1)               \n" // [1]
        :
        : "r" (font_data), "r" (gpiod_outdr)
        : "a2", "t0", "memory"
    );
}

// Render one active scanline (40 chars, called from generic IRQ handler)
// FrameBuf contains font indices (mapped by print.c), use directly
INLINE static void rca_render_line(u32 active_line) {
    (void)active_line; // unused, we track state internally

    const u8* text_row = current_text_row;
    const u8* font_row = current_font_row;

#if USE_RCA_SPI
    // SPI mode: graphics font format (1 byte = 8 pixels, direct to SPI)
    // Font layout: font[line * 256 + char_index], 2048 bytes total
    // ASM loop based on babypc_vga_asm.S VMODE 6 pattern
    //
    // SPI DIV4 @ 50MHz = 12.5 MHz SPI clock = 64 CPU cycles per byte
    // Loop body must take 64 cycles to match SPI timing
    asm volatile(
        "   li      a3, %[width]            \n" // loop counter = 40
        ".align 2,,                         \n" // align loop start
        "2:                                 \n" // main loop: 64 cycles per iteration
        "   lbu     a0, 0(%[text])          \n" // [2] load char index from FrameBuf
        "   addi    %[text], %[text], 1     \n" // [1] advance text pointer
        "   add     a0, %[font], a0         \n" // [1] compute font address
        "   lbu     a0, 0(a0)               \n" // [3] load font byte from Flash
        "   addi    a3, a3, -1              \n" // [1] decrement loop counter
        "   sw      a0, 0x0C(%[spi])        \n" // [2] write to SPI DATAR
        // delay loop: 4 iterations * 4 cycles = 16 cycles
        "   li      a2, 4                   \n" // [1] delay counter
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
#if USE_RCA_SPI
    current_font_row += 256; // SPI font: 256 bytes per line (font[line * 256 + char])
#else
    current_font_row += DrawFont->char_count * 4; // Nibble font: char_count * 4 bytes per line
#endif
    if (current_line_in_row >= LINES_PER_CHAR) {
        current_line_in_row = 0;
        current_font_row = DrawFont->data; // reset to first font line
        current_text_row += WIDTHBYTE; // advance to next text row
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

#endif // VMODE == 6
#endif // _RCA_VMODE_6_H
