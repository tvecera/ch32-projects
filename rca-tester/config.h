/**
 * MIT License
 *
 * Copyright (c) 2025 Tomas Vecera, tomas@vecera.dev
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
 * RCA Tester Configuration
 * ============================================================================
 *
 * Main configuration file for rca-tester application.
 * Defines video mode, clock source, and feature enables.
 *
 * ============================================================================
 */

#ifndef _CONFIG_H
#define _CONFIG_H

// ============================================================================
// Video Mode Selection
// ============================================================================
// Available video modes (B&W monochrome):
//
// Graphics modes (FrameBuf = pixels):
//   VMODE 0: 128x64 px,  RAM: 1024B buf + 306B handler = 1330B total
//   VMODE 1: 160x120 px, RAM: 2400B buf + 330B handler = 2730B total - Only for CH32V002
//
// Text modes (FrameBuf = ASCII characters, requires font in Flash):
//   VMODE 6: 40x30 chars (8x8 font),  RAM: 1200B buf + 20B state + 376B handler = 1596B
//   VMODE 7: 32x24 chars (8x8 font),  RAM: 768B buf + 20B state + 430B handler = 1218B
//   VMODE 8: 23x18 chars (8x14 font), RAM: 414B buf + 64B state + 418B handler = 896B
//
// Font Flash usage: 64 chars = 2048B, 128 chars = 4096B
#define VMODE           0

// Video output method:
//   USE_RCA_SPI = 0: Bit-banging on PD6 (default, works on all pins)
//   USE_RCA_SPI = 1: SPI1 MOSI on PC6 (hardware shift, frees CPU during pixel output)
//
// SPI mode requires PC6 for video output (SPI1_MOSI default pin)
// Bit-bang mode uses PD6 (can be changed in rca.c)
#define USE_RCA_SPI     1

// Clock source selection:
//   USE_HSI = 1: Internal HSI oscillator (no external crystal needed)
//                Less accurate, may cause slight timing drift
//   USE_HSI = 0: External HSE crystal oscillator (requires 25MHz crystal on PA1/PA2)
//                More accurate and stable timing for video sync
#define USE_HSI         0

// ============================================================================
// Application Features
// ============================================================================
#define USE_KEYS        1
#define USE_SOUND       2

// ============================================================================
// Font Selection
// ============================================================================
#if VMODE == 0 || VMODE == 1
#define FONT            FontBold8x8             // Graphics font (1KB or 2KB) for draw.c
// #else
// #define FONT            TextModeFontRvpc     // Text mode font descriptor (64 or 128 chars)
#endif

// ============================================================================
// Clock Configuration
// ============================================================================
#if USE_HSI
// frequency of HSI internal oscillator 25MHz
#define HSI_VALUE       25000000
// System clock source: 1=HSI, 2=HSE, 3=HSE_Bypass, 4=PLL_HSI, 5=PLL_HSE, 6=PLL_HSE_Bypass
#define SYSCLK_SRC      4

#else
// frequency of HSE external oscillator 25MHz
#define HSE_VALUE       25000000
// frequency of HSI internal oscillator 25MHz
#define HSI_VALUE       25000000
// System clock source: 1=HSI, 2=HSE, 3=HSE_Bypass, 4=PLL_HSI, 5=PLL_HSE, 6=PLL_HSE_Bypass
#define SYSCLK_SRC      5
#endif

// Only *2 supported; 25 MHz * 2 = 50 MHz
#define PLLCLK_MUL      2
// System clock divider: 1, 2, 3, 4, 5, 6, 7, 8, 16, 32, 64, 128, 256 (default 1)
#define SYSCLK_DIV      1

// ADC clock divider: (1,) 2, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128 (default 1 or 2)
#define ADCCLK_DIV      2                       // CH32V0: max. 24 MHz (48 / 2 = 24 MHz)

// number of HCLK clock cycles per 1 us (used with Wait functions)
// - If you want to change frequency of system clock run-time, use a variable instead of constant.
#define HCLK_PER_US     50

// increment of system time in [ms] on SysTick interrupt (0=do not use SysTick interrupt)
#define SYSTICK_MS      0

// ============================================================================
// Peripherals
// ============================================================================
#define ENABLE_SRAM     1                       // SRAM enable
#define ENABLE_FLASH    1                       // FLASH enable (needed to rewrite RESET function)
#define ENABLE_WWDG     0                       // Window watchdog enable
#define ENABLE_PWR      1                       // Power module enable
#define ENABLE_AFI      1                       // I/O auxiliary function enable
#define ENABLE_PA       1                       // PA port enable
#define ENABLE_PB       0                       // PB port enable
#define ENABLE_PC       1                       // PC port enable
#define ENABLE_PD       1                       // PD port enable
#define ENABLE_TIM1     1                       // TIM1 module enable
#define ENABLE_TIM2     1                       // TIM2 module enable
#if USE_RCA_SPI
#define ENABLE_SPI1     1                       // SPI1 module enable (required for SPI video output)
#else
#define ENABLE_SPI1     0                       // SPI1 module enable
#endif
#define ENABLE_USART1   0                       // USART1 module enable
#define ENABLE_I2C1     0                       // I2C1 module enable

#include "../rca-library/rca_config.h"

#endif //_CONFIG_H
