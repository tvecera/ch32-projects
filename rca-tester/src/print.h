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
 * Text Console Library for RCA Composite Video
 * ============================================================================
 * Provides terminal-style text output for text video modes (VMODE 6, 7, 8).
 *
 * Features:
 * - Cursor-based text output with automatic line wrapping
 * - Absolute positioning with print_char_at/print_text_at
 * - Automatic scrolling when cursor reaches bottom
 * - Font mapping: ASCII to font index conversion
 *
 * Text dimensions depend on VMODE:
 * - VMODE 6: 40x30 characters
 * - VMODE 7: 32x24 characters
 * - VMODE 8: 23x18 characters
 *
 * Font mapping:
 * - FrameBuf contains font indices (0-63 or 0-127), NOT raw ASCII
 * - Mapping is done here in print functions (not in IRQ handler)
 * - Standard fonts use fast math, custom fonts use lookup table
 * - See font_desc.h for font descriptor structure
 *
 * ============================================================================
 */

#if VMODE > 1

#ifndef _PRINT_H
#define _PRINT_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Cursor State Variables
// ============================================================================
extern int print_pos;   // Current cursor column (0..TEXTWIDTH-1)
extern int print_row;   // Current cursor row (0..TEXTHEIGHT-1)

// ============================================================================
// Current Font
// ============================================================================

extern const font_desc_t* fontdata;

// ============================================================================
// Console Functions
// ============================================================================

/**
 * Clear screen and reset cursor to top-left
 * Fills FrameBuf with space characters (0x20)
 */
void print_clear();

/**
 * Print character at absolute position
 * @param ch Character to print
 * @param x  Column (0..TEXTWIDTH-1)
 * @param y  Row (0..TEXTHEIGHT-1)
 */
void print_char_at(char ch, int x, int y);

/**
 * Print string at absolute position
 * @param text Null-terminated string
 * @param x    Starting column
 * @param y    Row
 */
void print_text_at(const char* text, int x, int y);

/**
 * Reset cursor to top-left (0,0)
 * Does not clear screen
 */
void print_reset();

/**
 * Move cursor to beginning of next line
 * Scrolls screen if at bottom row
 */
void print_newline();

/**
 * Scroll screen up by one line
 * Top line is discarded, bottom line is cleared
 */
void print_scroll();

/**
 * Print character at cursor position
 * Advances cursor, wraps to next line if needed
 * @param ch Character to print
 */
void print_char(char ch);

/**
 * Print string at cursor position
 * Advances cursor, wraps and scrolls as needed
 * @param text Null-terminated string
 */
void print_text(const char* text);

/**
 * Set active font and reinitialize video driver state
 * Use this function to change fonts at runtime
 * @param font Pointer to font descriptor
 */
void print_set_font(const font_desc_t* font);

/**
 * Map ASCII character to font index
 *
 * For standard 128-char fonts (ascii_map == NULL):
 *   Uses fast math: ch & 0x7F
 *   ~2 cycles
 *
 * For 64-char or custom fonts (ascii_map != NULL):
 *   Uses lookup table: ascii_map[ch & 0x7F]
 *   ~3-4 cycles
 *
 * @param font  Pointer to font descriptor
 * @param ch    ASCII character to map
 * @return      Font index (0 to char_count-1)
 */
__attribute__((always_inline)) static inline u8 font_map(const font_desc_t* font, char ch) {
    if (font->ascii_map) {
        // 64-char or custom font - use lookup table
        return font->ascii_map[(u8)ch & 0x7F];
    }
    // Standard 128-char font - fast math
    return (u8)ch & 0x7F;
}

#ifdef __cplusplus
}
#endif

#endif // _PRINT_H

#endif // VMODE > 1
