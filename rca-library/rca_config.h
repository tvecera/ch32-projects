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
 * RCA Library Default Configuration
 * ============================================================================
 *
 * Default configuration values for rca-library.
 * These can be overridden by defining values before including this file.
 *
 * ============================================================================
 */

#ifndef _RCA_CONFIG_H
#define _RCA_CONFIG_H

// ============================================================================
// Video Mode Selection
// ============================================================================
// Available video modes (B&W monochrome):
//
// Graphics modes (FrameBuf = pixels):
//   VMODE 0: 128x64 px,  RAM: 1024B buf + 306B handler = 1330B total
//   VMODE 1: 160x120 px, RAM: 2400B buf + 330B handler = 2730B total
//
// Text modes (FrameBuf = ASCII characters, requires font in Flash):
//   VMODE 6: 40x30 chars (8x8 font),  RAM: 1200B buf + 20B state + 376B handler = 1596B
//   VMODE 7: 32x24 chars (8x8 font),  RAM: 768B buf + 20B state + 430B handler = 1218B
//   VMODE 8: 23x18 chars (8x14 font), RAM: 414B buf + 64B state + 418B handler = 896B
//
// Font Flash usage: 64 chars = 2048B, 128 chars = 4096B
#ifndef VMODE
#define VMODE           1
#endif

// Video output method:
//   USE_RCA_SPI = 0: Bit-banging on PD6 (default, works on all pins)
//   USE_RCA_SPI = 1: SPI1 MOSI on PC6 (hardware shift, frees CPU during pixel output)
//
// SPI mode requires PC6 for video output (SPI1_MOSI default pin)
// Bit-bang mode uses PD6 (can be changed in rca.c)
#ifndef USE_RCA_SPI
#define USE_RCA_SPI     0
#endif

// Clock source selection:
//   USE_HSI = 1: Internal HSI oscillator (no external crystal needed)
//                Less accurate, may cause slight timing drift
//   USE_HSI = 0: External HSE crystal oscillator (requires 25MHz crystal on PA1/PA2)
//                More accurate and stable timing for video sync
#ifndef USE_HSI
#define USE_HSI         0
#endif

#if VMODE == 0 || VMODE == 1
#ifndef FONT
#define FONT            FontBold8x8             // Graphics font (1KB or 2KB) for draw.c
#endif
#else
#ifndef FONT
#define FONT            TextModeFontBold8x8     // Text mode font descriptor (64 or 128 chars)
#endif
#endif

#if VMODE > 5
#ifndef FONT
    #error "FONT not defined!"
#endif
#endif

#if USE_HSI
// frequency of HSI internal oscillator 25MHz
#ifndef HSI_VALUE
#define HSI_VALUE       25000000
#endif

// System clock source: 1=HSI, 2=HSE, 3=HSE_Bypass, 4=PLL_HSI, 5=PLL_HSE, 6=PLL_HSE_Bypass
#ifndef SYSCLK_SRC
#define SYSCLK_SRC      4
#endif

#else
// frequency of HSE external oscillator 25MHz
#ifndef HSE_VALUE
#define HSE_VALUE       25000000
#endif

// frequency of HSI internal oscillator 25MHz
#ifndef HSI_VALUE
#define HSI_VALUE       25000000
#endif

// System clock source: 1=HSI, 2=HSE, 3=HSE_Bypass, 4=PLL_HSI, 5=PLL_HSE, 6=PLL_HSE_Bypass
#ifndef SYSCLK_SRC
#define SYSCLK_SRC      5
#endif

#endif

// Only *2 supported; 25 MHz * 2 = 50 MHz
#ifndef PLLCLK_MUL
#define PLLCLK_MUL      2
#endif

// System clock divider: 1, 2, 3, 4, 5, 6, 7, 8, 16, 32, 64, 128, 256 (default 1)
#ifndef SYSCLK_DIV
#define SYSCLK_DIV      1
#endif

// ADC clock divider: (1,) 2, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128 (default 1 or 2)
#ifndef ADCCLK_DIV
#define ADCCLK_DIV      2                       // CH32V0: max. 24 MHz (48 / 2 = 24 MHz)
#endif

// number of HCLK clock cycles per 1 us (used with Wait functions)
// - If you want to change frequency of system clock run-time, use a variable instead of constant.
#ifndef HCLK_PER_US
#define HCLK_PER_US     50
#endif

// increment of system time in [ms] on SysTick interrupt (0=do not use SysTick interrupt)
#ifndef SYSTICK_MS
#define SYSTICK_MS      0
#endif

// ============================================================================
// Peripherals
// ============================================================================
#ifndef ENABLE_SRAM
#define ENABLE_SRAM     1                       // SRAM enable
#endif

#ifndef ENABLE_FLASH
#define ENABLE_FLASH    1                       // FLASH enable (needed to rewrite RESET function)
#endif

#ifndef ENABLE_WWDG
#define ENABLE_WWDG     0                       // Window watchdog enable
#endif

#ifndef ENABLE_PWR
#define ENABLE_PWR      1                       // Power module enable
#endif

#ifndef ENABLE_AFI
#define ENABLE_AFI      1                       // I/O auxiliary function enable
#endif

#ifndef ENABLE_PA
#define ENABLE_PA       1                       // PA port enable
#endif

#ifndef ENABLE_PB
#define ENABLE_PB       0                       // PB port enable
#endif

#ifndef ENABLE_PC
#define ENABLE_PC       1                       // PC port enable
#endif

#ifndef ENABLE_PD
#define ENABLE_PD       1                       // PD port enable
#endif

#ifndef ENABLE_TIM1
#define ENABLE_TIM1     1                       // TIM1 module enable
#endif

#ifndef ENABLE_TIM2
#define ENABLE_TIM2     1                       // TIM2 module enable
#endif

#ifndef ENABLE_SPI1
#if USE_RCA_SPI
#define ENABLE_SPI1     1                       // SPI1 module enable (required for SPI video output)
#else
#define ENABLE_SPI1     0                       // SPI1 module enable
#endif
#endif

#ifndef ENABLE_USART1
#define ENABLE_USART1   0                       // USART1 module enable
#endif

#ifndef ENABLE_I2C1
#define ENABLE_I2C1     0                       // I2C1 module enable
#endif

// CH32Fun specific configuration
#define FUNCONF_ENABLE_HPE       0
#define FUNCONF_USE_DEBUGPRINTF  0

#endif // _RCA_CONFIG_H
