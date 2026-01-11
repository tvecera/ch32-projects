#ifndef _FUNCONFIG_H
#define _FUNCONFIG_H

#include "config.h"

// System clock source: 1=HSI, 2=HSE, 3=HSE_Bypass, 4=PLL_HSI, 5=PLL_HSE, 6=PLL_HSE_Bypass
#if SYSCLK_SRC == 1
#define FUNCONF_USE_HSI    1  // Use HSI Internal Oscillator
#define FUNCONF_USE_HSE    0  // Use External Oscillator
#define FUNCONF_USE_PLL    0  // No PLL - use HSI directly
#define FUNCONF_HSE_BYPASS 0  // Use HSE Bypass feature (for oscillator input)
#elif SYSCLK_SRC == 2
#define FUNCONF_USE_HSI    0  // Use HSI Internal Oscillator
#define FUNCONF_USE_HSE    1  // Use External Oscillator
#define FUNCONF_USE_PLL    0  // No PLL - use HSI directly
#define FUNCONF_HSE_BYPASS 0  // Use HSE Bypass feature (for oscillator input)
#elif SYSCLK_SRC == 3
#define FUNCONF_USE_HSI    0  // Use HSI Internal Oscillator
#define FUNCONF_USE_HSE    1  // Use External Oscillator
#define FUNCONF_USE_PLL    0  // No PLL - use HSI directly
#define FUNCONF_HSE_BYPASS 1  // Use HSE Bypass feature (for oscillator input)
#elif SYSCLK_SRC == 4
#define FUNCONF_USE_HSI    1  // Use HSI Internal Oscillator
#define FUNCONF_USE_HSE    0  // Use External Oscillator
#define FUNCONF_USE_PLL    1  // Use PLL to multiply clock
#define FUNCONF_HSE_BYPASS 0  // Use HSE Bypass feature (for oscillator input)
#elif SYSCLK_SRC == 5
#define FUNCONF_USE_HSI    0  // Use HSI Internal Oscillator
#define FUNCONF_USE_HSE    1  // Use External Oscillator
#define FUNCONF_USE_PLL    1  // Use PLL to multiply clock
#define FUNCONF_HSE_BYPASS 0  // Use HSE Bypass feature (for oscillator input)
#elif SYSCLK_SRC == 6
#define FUNCONF_USE_HSI    0  // Use HSI Internal Oscillator
#define FUNCONF_USE_HSE    1  // Use External Oscillator
#define FUNCONF_USE_PLL    1  // Use PLL to multiply clock
#define FUNCONF_HSE_BYPASS 1  // Use HSE Bypass feature (for oscillator input)
#endif

#define FUNCONF_PLL_MULTIPLIER   PLLCLK_MUL
#define FUNCONF_USE_CLK_SEC	     0
#define FUNCONF_SYSTICK_USE_HCLK 1

#endif
