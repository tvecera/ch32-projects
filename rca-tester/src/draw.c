/**
 * ============================================================================
 * Graphics Drawing Library for Monochrome Framebuffer
 * ============================================================================
 *
 * Based on drawing functions from PicoLibSDK / CH32LibSDK
 * Original: https://github.com/Panda381/PicoLibSDK
 *           https://github.com/Panda381/CH32LibSDK
 * Author:   Miroslav Nemecek
 *
 * See draw.h for API documentation.
 *
 * ============================================================================
 */

#include "../include.h"

void DrawClear() {
#if VMODE == 0 || VMODE == 1
    memset(FrameBuf, 0x00, FRAMESIZE);
#else
    print_clear();
#endif
}

#if VMODE == 0 || VMODE == 1

NOINLINE void DrawPointFast(int x, int y, u8 col) {
    u8* p = FrameBuf + (x >> 3) + y * WIDTHBYTE;
    u8 bit = x & 7;

    if (col) {
        *p |= 0x80 >> bit;
    } else {
        *p &= ~(0x80 >> bit);
    }
}

NOINLINE void DrawPointClrFast(int x, int y) {
    // clear pixel
    u8* d = &FrameBuf[(x >> 3) + y * WIDTHBYTE];
    x = 7 - (x & 7);
    *d &= ~(1 << x);
}

void DrawPoint(int x, int y, u8 col) {
    if ((x >= 0) && (x < WIDTH) && (y >= 0) && (y < HEIGHT)) DrawPointFast(x, y, col);
}

void DrawPointClr(int x, int y) {
    if ((x >= 0) && (x < WIDTH) && (y >= 0) && (y < HEIGHT)) DrawPointClrFast(x, y);
}

#ifdef FONT

const u8* volatile DrawFont = FONT;
u8 PrintInv = 0;

void DrawChar(char ch, int x, int y, u8 col) {
    const u8* src = &DrawFont[(u8)ch];
    int i, j;
    u8 m;
    for (i = 8; i > 0; i--) {
        m = *src;
        if (PrintInv != 0) m = ~m;
        for (j = 8; j > 0; j--) {
            if ((m & (1 << 7)) != 0)
                DrawPoint(x, y, col);
            else
                DrawPointClr(x, y);
            m <<= 1;
            x++;
        }
        x -= 8;
        y++;
        src += 256;
    }
}

void DrawText(const char* text, int x, int y, u8 col) {
    char ch;
    while ((ch = *text++) != 0) {
        DrawChar(ch, x, y, col);
        x += 8;
    }
}

#endif // FONT

#endif // VMODE == 0 || VMODE == 1
