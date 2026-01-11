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
#include "../patterns/patterns.h"

// Pattern switch interval in ms
#define LOOP_WAIT           2
#define PATTERN_INTERVAL_MS (LOOP_WAIT * 1000)

void frame_callback() {
#if USE_KEYS
    KeyScan();
#endif

#if USE_SOUND
    SoundScan();
#endif
}

void DevInit() {
#ifdef CH32FUN
    SystemInit();
    funGpioInitAll();

    SysTick->CTLR &= ~(1 << 0);
    SysTick->CTLR &= ~(1 << 1);
    SysTick->CTLR |= (1 << 2);
    SysTick->CTLR &= ~(1UL << 31);
    SysTick->CNT = 0x00000000;
    SysTick->CMP = HCLK_PER_MS / 2 - 1;
    SysTick->SR &= ~(1 << 0);
    SysTick->CTLR &= ~(1 << 3);
    SysTick->CTLR |= (1 << 0);
#endif

#if USE_KEYS || USE_SOUND
    rca_set_frame_callback(frame_callback);
#endif
    rca_init();

#if USE_KEYS
    KeyInit();
#endif

#if USE_SOUND
    SoundInit();
#endif
}

void DevTerm() {
#if USE_KEYS
    KeyWaitNoPressed();
#endif

    rca_stop();

#if USE_SOUND
    SoundTerm();
#endif

#if USE_KEYS
    KeyTerm();
#endif
}

#if VMODE == 0 || VMODE == 1

// Load pattern from Flash to framebuffer (fast memcpy)
void draw_pattern(u8 pattern) {
    if (pattern < PATTERN_COUNT) {
        if (pattern == PATTERN_TEXT) {
            DrawClear();
            pattern_text();
        } else if (pattern == PATTERN_CHARSET) {
            DrawClear();
            pattern_charset();
        } else {
            LoadPattern(pattern);
        }
    }
}

int main() {
    u8 pattern = 0;

#ifdef CH32FUN
DevInit();
#endif

rca_wait_vsync();
draw_pattern(pattern);

char ch = 0;

    while (1) {
        ch = KeyGet();
        if (ch == NOKEY) {
            // Wait for next frame to reduce CPU usage
            rca_wait_vsync();
            continue;
        }

        if (ch == KEY_RIGHT) {
            PlayMelody(SoundSamp1);
            pattern = (pattern + 1) % PATTERN_COUNT;
            rca_wait_vsync();
            draw_pattern(pattern);
        } else if (ch == KEY_LEFT) {
             PlayMelody(SoundSamp1);
             pattern = (pattern + PATTERN_COUNT - 1) % PATTERN_COUNT;
             rca_wait_vsync();
             draw_pattern(pattern);
        } else if (ch == KEY_UP) {
             ResetToBootLoader();
        }
    }
}
#else

#define FONTS_COUNT 2

void show_font(const char* name) {
    print_clear();
    int y = TEXTHEIGHT / 2;
    int len = 0;
    const char* p = name;
    while (*p++) len++;
    int x = (TEXTWIDTH - len) / 2;
    if (x < 0) x = 0;
    print_text_at(name, x, y);
	while (KeyGet() == NOKEY) {}
    print_clear();
    for (int i = 0; i < FRAMESIZE - 2; i++) {
        print_char((char)(i % DrawFont->char_count));
    }
}

NOINLINE void choose_font(const u8 idx) {
    switch (idx) {
         case 0:
            print_set_font(&TextModeFontBold8x8);
            show_font("TextModeFontBold8x8");
            break;
        case 1:
            print_set_font(&TextModeFontThin8x8);
            show_font("TextModeFontThin8x8");
            break;
        // case 0:
        //     print_set_font(&TextModeFontRvpc);
        //     show_font("TextModeFontRvpc");
        //     break;
        // case 1:
        //     print_set_font(&TextModeFont80);
        //     show_font("TextModeFont80");
        //     break;
        // case 0:
        //     print_set_font(&TextModeFontBold8x8_64);
        //     show_font("TextModeFontBold8x8_64");
        //     break;
        // case 1:
        //     print_set_font(&TextModeFontThin8x8_64);
        //     show_font("TextModeFontThin8x8_64");
        //     break;
        // case 2:
        //     print_set_font(&TextModeFontZx);
        //     show_font("TextModeFontZx");
        //     break;
        // case 3:
        //     print_set_font(&TextModeFont81);
        //     show_font("TextModeFont81");
        //     break;
        default:
            break;
    }
}

int main() {
#ifdef CH32FUN
    DevInit();
#endif

    rca_wait_vsync();
    print_clear();

    u8 font_idx = 0;
    choose_font(font_idx);

    char ch = 0;

    while (1) {
        ch = KeyGet();
        if (ch == NOKEY) {
            // Wait for next frame to reduce CPU usage
            rca_wait_vsync();
            continue;
        }

        if (ch == KEY_RIGHT) {
            PlayMelody(SoundSamp1);
            font_idx = (font_idx + 1) % FONTS_COUNT;
            rca_wait_vsync();
            choose_font(font_idx);
        } else if (ch == KEY_LEFT) {
            PlayMelody(SoundSamp1);
            font_idx = (font_idx + FONTS_COUNT - 1) % FONTS_COUNT;
            rca_wait_vsync();
            choose_font(font_idx);
        } else if (ch == KEY_UP) {
            ResetToBootLoader();
        }
    }
}
#endif
