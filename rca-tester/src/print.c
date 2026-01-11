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
 */
#include "../include.h"

#if VMODE > 1

int print_pos = 0;
int print_row = 0;

const font_desc_t* DrawFont = &FONT;

/**
 * Clear screen and reset cursor
 * Fills FrameBuf with space character (font index for ' ')
 */
void print_clear() {
    memset(FrameBuf, DrawFont->space_idx, FRAMESIZE);
    print_pos = 0;
    print_row = 0;
}

/**
 * Scroll screen up by one line
 */
void print_scroll() {
    if (print_row > 0) {
        print_row--;
    }
    memmove(&FrameBuf[0], &FrameBuf[WIDTHBYTE], FRAMESIZE - WIDTHBYTE);
    memset(&FrameBuf[FRAMESIZE - WIDTHBYTE], DrawFont->space_idx, WIDTHBYTE);
}

/**
 * Print character at absolute position
 * Maps ASCII to font index before storing in FrameBuf
 */
void print_char_at(const char ch, const int x, const int y) {
    if ((x < 0) || (x >= TEXTWIDTH) || (y < 0) || (y >= TEXTHEIGHT)) return;
    FrameBuf[WIDTHBYTE * y + x] = font_map(DrawFont, ch);
}

void print_text_at(const char* text, int x, const int y) {
    char ch;
    while ((ch = *text++) != 0) {
        print_char_at(ch, x, y);
        x++;
    }
}

void print_reset() {
    print_pos = 0;
    print_row = 0;
}

void print_newline() {
    print_pos = 0;
    print_row++;

    if (print_row >= TEXTHEIGHT) {
        print_scroll();
        print_row = TEXTHEIGHT - 1;
    }
}

/**
 * Print character at cursor position
 * Handles special characters:
 *   '\n' (0x0A) - newline (move to start of next line)
 *   '\r' (0x0D) - carriage return (move to start of current line)
 */
void print_char(const char ch) {
    if (ch == '\n') {
        print_newline();
    } else if (ch == '\r') {
        print_pos = 0;
    } else {
        // Bounds check before array access
        if (print_row >= TEXTHEIGHT) {
            print_newline();
        }
        FrameBuf[print_row * WIDTHBYTE + print_pos] = font_map(DrawFont, ch);
        if (++print_pos >= TEXTWIDTH) print_newline();
    }
}

void print_text(const char* text) {
    char ch;
    while ((ch = *text++) != 0) {
        print_char(ch);
    }
}

void print_set_font(const font_desc_t* font) {
    if (font == 0) return;
    DrawFont = font;
    rca_update_font();
}

#endif // VMODE > 1
