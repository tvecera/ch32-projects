
// Videomodes (B&W mono):
//  0 ... graphics mode 128x64 pixels, required memory 1024 B (driver size 738 B in RAM) - SPI_BAUD_DIV8	// SPI horizontal divider
//  1 ... graphics mode 160x120 pixels, required memory 2400 B (driver size 738 B in RAM) - SPI_BAUD_DIV8	// SPI horizontal divider
//  8 ... text mode 23x18 characters of 8x8 pixels with custom fonts of 64/128 characters - 720x576 pixels (PAL),
//	required memory 414 bytes, font 8x8 pixels 4096/8192 bytes in Flash
#ifndef VMODE
#define VMODE	1
#endif

#ifndef FONT_CHAR_COUNT
#define FONT_CHAR_COUNT	128
#endif

#ifndef SOUND_MUTE
#define SOUND_MUTE   0   // 1=MUTE sound
#endif

// default device setup
#ifndef USE_DRAW
#define USE_DRAW	1	// 1=use graphics drawing functions
#endif

#ifndef USE_PRINT
#define USE_PRINT	1	// 1=use text printing functions
#endif

#ifndef USE_KEYS
#define USE_KEYS	1	// 1=use keyboard support
#endif

// Fix problem with sdk_runtime
#ifndef USE_KEY
#define USE_KEY		0	// 1=use keyboard support
#endif

#ifndef USE_SOUND
#define USE_SOUND	1	// 1=use sound support
#endif

#ifndef USE_DISP
#define USE_DISP	1	// 1=use display support
#endif

#ifndef USE_RCA
#define USE_RCA	    0
#endif

// ----------------------------------------------------------------------------
//                            Clock Setup
// ----------------------------------------------------------------------------

// frequency of HSI internal oscillator 24MHz
#ifndef HSI_VALUE
#define HSI_VALUE	25000000
#endif

// System clock source: 1=HSI, 2=HSE, 3=HSE_Bypass, 4=PLL_HSI, 5=PLL_HSE, 6=PLL_HSE_Bypass, 7=PLL_HSI/2, 8=PLL_HSE/2, 9=PLL_HSE_Bypass/2
#ifndef SYSCLK_SRC
#define SYSCLK_SRC	4
#endif

// PLL multiplier
#ifndef PLLCLK_MUL
#define PLLCLK_MUL	2		// only *2 supported; 24 MHz * 2 = 48 MHz
#endif

// System clock divider: 1, 2, 3, 4, 5, 6, 7, 8, 16, 32, 64, 128, 256 (default 1)
#ifndef SYSCLK_DIV
#define SYSCLK_DIV	1
#endif

// ADC clock divider: (1,) 2, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128 (default 1 or 2)
#ifndef ADCCLK_DIV
#define ADCCLK_DIV	2		// CH32V0: max. 24 MHz (48 / 2 = 24 MHz)
#endif

// number of HCLK clock cycles per 1 us (used with Wait functions)
// - If you want to change frequency of system clock run-time, use a variable instead of constant.
#ifndef HCLK_PER_US
#define HCLK_PER_US	50
#endif

// increment of system time in [ms] on SysTick interrupt (0=do not use SysTick interrupt)
#ifndef SYSTICK_MS
#define SYSTICK_MS	0
#endif

// ----------------------------------------------------------------------------
//                          Peripheral clock enable
// ----------------------------------------------------------------------------

// Periferals
#ifndef ENABLE_SRAM
#define ENABLE_SRAM	1		// SRAM enable
#endif

#ifndef ENABLE_FLASH
#define ENABLE_FLASH	1		// FLASH enable (needed to rewrite RESET function)
#endif

#ifndef ENABLE_WWDG
#define ENABLE_WWDG	0		// Window watchdog enable
#endif

#ifndef ENABLE_PWR
#define ENABLE_PWR	1		// Power module enable
#endif

#ifndef ENABLE_AFI
#define ENABLE_AFI	1		// I/O auxiliary function enable
#endif

#ifndef ENABLE_PA
#define ENABLE_PA	1		// PA port enable
#endif

#ifndef ENABLE_PB
#define ENABLE_PB	0		// PB port enable
#endif

#ifndef ENABLE_PC
#define ENABLE_PC	1		// PC port enable
#endif

#ifndef ENABLE_PD
#define ENABLE_PD	1		// PD port enable
#endif

#ifndef ENABLE_TIM1
#define ENABLE_TIM1	1		// TIM1 module enable
#endif

#ifndef ENABLE_TIM2
#define ENABLE_TIM2	1		// TIM2 module enable
#endif

#ifndef ENABLE_SPI1
#define ENABLE_SPI1	0		// SPI1 module enable
#endif

#ifndef ENABLE_USART1
#define ENABLE_USART1	0		// USART1 module enable
#endif

#ifndef ENABLE_I2C1
#define ENABLE_I2C1	0		// I2C1 module enable
#endif
