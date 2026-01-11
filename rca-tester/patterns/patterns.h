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
#ifndef _PATTERNS_H
#define _PATTERNS_H

#include "../config.h"

#if VMODE == 0 || VMODE == 1

#define PATTERN_GRID         0
#define PATTERN_CROSSHATCH   1
#define PATTERN_CIRCLES      2
#define PATTERN_BORDER       3
#define PATTERN_CHECKERBOARD 4
#define PATTERN_DOTS         5
#define PATTERN_TEXT         6
#define PATTERN_CHARSET      7
#define PATTERN_COUNT        8

// Pattern indices
#if VMODE == 0
#include "patterns_vmode0.h"
#elif VMODE == 1
#include "patterns_vmode1.h"

// RLE decompression for VMODE 1
// Format: MSB=1 (0x80-0xFF): repeat next byte (count-0x80+3) times
//         MSB=0 (0x00-0x7F): next (count+1) bytes are literal
INLINE void rle_decompress(const unsigned char* src, unsigned char* dst, unsigned short dst_size) {
    unsigned short dst_pos = 0;
    while (dst_pos < dst_size) {
        unsigned char ctrl = *src++;
        if (ctrl & 0x80) {
            // Repeat run: repeat next byte (ctrl - 0x80 + 3) times
            unsigned char count = (ctrl & 0x7F) + 3;
            unsigned char value = *src++;
            while (count-- && dst_pos < dst_size) {
                dst[dst_pos++] = value;
            }
        } else {
            // Literal run: copy (ctrl + 1) bytes
            unsigned char count = ctrl + 1;
            while (count-- && dst_pos < dst_size) {
                dst[dst_pos++] = *src++;
            }
        }
    }
}
#endif

// Pattern lookup table (NULL for disabled patterns)
static const unsigned char* const PatternTable[PATTERN_COUNT - 2] = {
    pattern_grid,
    pattern_crosshatch,
    pattern_circles,
    pattern_border,
    pattern_checkerboard,
    pattern_dots,
};

/**
 * Load pattern from Flash to FrameBuf with NULL check
 * @param idx Pattern index (0 to PATTERN_COUNT-3)
 * @return 1 if pattern loaded, 0 if pattern is disabled (NULL)
 */
static inline u8 LoadPattern(u8 idx) {
    if (idx >= (PATTERN_COUNT - 2)) return 0;
    if (PatternTable[idx] == 0) return 0;
#if VMODE == 0
    memcpy(FrameBuf, PatternTable[idx], FRAMESIZE);
#elif VMODE == 1
    rle_decompress(PatternTable[idx], FrameBuf, FRAMESIZE);
#endif
    return 1;
}

#ifdef FONT
// Text rendering test
INLINE void pattern_text() {
    DrawText("CRT Test Pattern", 0, 0, COL_WHITE);
    DrawText("ABCDEFGHIJKLMNOP", 0, 10, COL_WHITE);
    DrawText("abcdefghijklmnop", 0, 20, COL_WHITE);
    DrawText("0123456789!@#$%^", 0, 30, COL_WHITE);
    DrawText("CRT Test Pattern", 0, 40, COL_WHITE);
    DrawText("CRT Test Pattern", 0, 50, COL_WHITE);
}

// Full character set display
INLINE void pattern_charset() {
    int x = 0;
    int y = 0;
    for (char c = 32; c < 128; c++) {
        DrawChar(c, x, y, COL_WHITE);
        x += 8;
        if (x >= WIDTH - 8) {
            x = 0;
            y += 9;
            if (y >= HEIGHT - 8) return;
        }
    }
}

#endif

#endif // VMODE == 0 || VMODE == 1

#endif // _PATTERNS_H
