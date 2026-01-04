/**
 * ============================================================================
 * CH32LibSDK Compatibility Layer for ch32fun Framework
 * ============================================================================
 *
 * Based on CH32LibSDK by Miroslav Nemecek (Panda38)
 * https://github.com/Panda381/CH32LibSDK
 *
 * Provides type definitions and macros to allow CH32LibSDK-style code to
 * compile with the ch32fun framework. Maps CH32LibSDK API calls to their
 * ch32fun equivalents.
 *
 * ============================================================================
 */

#ifndef _CH32LIBSDK_H
#define _CH32LIBSDK_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long int u32;
typedef unsigned long long int u64;

typedef unsigned char Bool;
#define True 1
#define False 0

// Bit definitions
#define B0 (1 << 0)
#define B1 (1 << 1)
#define B2 (1 << 2)
#define B3 (1 << 3)
#define B4 (1 << 4)
#define B5 (1 << 5)
#define B6 (1 << 6)
#define B7 (1 << 7)

#define cb() __asm volatile ("" ::: "memory")
#define nop() __asm volatile (" nop\n" ::: "memory")

// request to use inline
#define INLINE __attribute__((always_inline)) inline
// avoid to use inline
#define NOINLINE __attribute__((noinline))
// align array to 4-bytes
#define ALIGNED __attribute__((aligned(4)))

#define WaitMs(ms) Delay_Ms(ms)

#define RCC_HSITrim(trim) RCC->CTLR = ((RCC->CTLR & ~RCC_HSITRIM) | (trim << 3))
#define RCC_PCClkEnable() funGpioInitC()
#define RCC_PDClkEnable() funGpioInitD()
#define RCC_AFIClkEnable() RCC->APB2PCENR |= (1<<0); cb()

#define GPIO_MODE_AF_FAST (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)
#define GPIO_MODE_AF (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)
#define GPIO_MODE_OUT_FAST (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)
#define GPIO_MODE_IN_PU GPIO_CNF_IN_PUPD
#define GPIO_MODE_IN GPIO_CNF_IN_FLOATING

#define GPIO_PinReset(gpio) funPinMode(gpio, GPIO_CNF_IN_FLOATING)
#define GPIO_Mode(gpio, mode) funPinMode(gpio, mode)
#define GPIO_Out1(gpio) funDigitalWrite(gpio, 1)
#define GPIO_Out0(gpio) funDigitalWrite(gpio, 0)
#define GPIO_In(gpio) funDigitalRead(gpio)


#define TIM_INMODE_INT          ((uint16_t)0x0000)  // Internal clock (SMS=0, no slave mode)
#define TIM_COMP_PWM1           (TIM_OCMode_PWM1 >> 4)  // = 6, PWM mode 1

#define TIM_COMP_FREEZE TIM_OC2PE
#define TIM1_ClkEnable() RCC->APB2PCENR |= RCC_APB2Periph_TIM1; cb()
#define TIM1_Reset() RCC->APB2PRSTR |= RCC_APB2Periph_TIM1; cb(); nop(); RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1; cb()
#define TIM1_Div(div) TIM1->PSC = (div) - 1
#define TIM1_Load(val) TIM1->ATRLR = (val)
#define TIM1_Comp4(val) TIM1->CH4CVR = (val)
#define TIM1_DirUp() TIM1->CTLR1 &= ~TIM_DIR
#define TIM1_Update() TIM1->SWEVGR |= TIM_UG
#define TIM1_InMode(inmode) TIM1->SMCFGR = (TIM1->SMCFGR & ~TIM_SMS) | (inmode)
#define TIM1_AutoReloadEnable() TIM1->CTLR1 |= TIM_ARPE
#define TIM1_OC4Mode(mode) TIM1->CHCTLR2 = (TIM1->CHCTLR2 & ~TIM_OC4M) | ((mode) << 12)
#define TIM1_OC4PreEnable() TIM1->CHCTLR2 |= TIM_OC4PE
#define TIM1_OCEnable() TIM1->BDTR |= TIM_MOE
#define TIM1_Enable() TIM1->CTLR1 |= TIM_CEN
#define TIM1_Disable() TIM1->CTLR1 &= ~TIM_CEN
#define TIM1_CC4Enable() TIM1->CCER |= TIM_CC4E
#define TIM1_CC4Disable() TIM1->CCER &= ~TIM_CC4E

#ifdef CH32V003
#define GPIO_Remap_TIM1(map) AFIO->PCFR1 = (AFIO->PCFR1 & ~(0x3 << 6)) | ((map) << 6)
#else
#define GPIO_Remap_TIM1(map) AFIO->PCFR1 = (AFIO->PCFR1 & ~(0xF << 10)) | ((map) << 10)
#endif
#ifdef CH32V003
#define GPIO_Remap_TIM2(map) AFIO->PCFR1 = (AFIO->PCFR1 & ~AFIO_PCFR1_TIM2_REMAP) | ((map) << 8)
#else
#define GPIO_Remap_TIM2(map) AFIO->PCFR1 = (AFIO->PCFR1 & ~AFIO_PCFR1_TIM2_RM) | ((map) << 14)
#endif
#define IRQ_TIM2 TIM2_IRQn
#define NVIC_IRQEnable(irq) NVIC_EnableIRQ(irq)
#define NVIC_IRQDisable(irq) NVIC_DisableIRQ(irq)

#define SystemReset() NVIC->CFGR = NVIC_KEY3 | (1<<7)
#define ResetToBootLoader() SystemReset()

#ifndef HCLK_PER_MS
#define HCLK_PER_MS (HCLK_PER_US*1000)
#endif
#define Time() (SysTick->CNT)

// ============================================================================
// SPI1 Functions
// ============================================================================
// SPI Baud Rate Prescaler (BR bits in CTLR1, bits 3-5)
#define SPI_BAUD_DIV2   (0 << 3)  // fPCLK/2
#define SPI_BAUD_DIV4   (1 << 3)  // fPCLK/4
#define SPI_BAUD_DIV8   (2 << 3)  // fPCLK/8
#define SPI_BAUD_DIV16  (3 << 3)  // fPCLK/16
#define SPI_BAUD_DIV32  (4 << 3)  // fPCLK/32
#define SPI_BAUD_DIV64  (5 << 3)  // fPCLK/64
#define SPI_BAUD_DIV128 (6 << 3)  // fPCLK/128
#define SPI_BAUD_DIV256 (7 << 3)  // fPCLK/256

// RCC Clock Enable/Reset for SPI1
#define RCC_SPI1ClkEnable()  do { RCC->APB2PCENR |= RCC_APB2Periph_SPI1; cb(); } while(0)
#define RCC_SPI1Reset()      do { RCC->APB2PRSTR |= RCC_APB2Periph_SPI1; cb(); nop(); RCC->APB2PRSTR &= ~RCC_APB2Periph_SPI1; cb(); } while(0)

// SPI1 Configuration Macros
// CTLR1 bit positions: BR[5:3], MSTR[2], SSM[9], SSI[8], LSBFIRST[7], DFF[11], SPE[6], BIDIMODE[15], BIDIOE[14]
#define SPI1_Baud(baud)     SPI1->CTLR1 = (SPI1->CTLR1 & ~(7 << 3)) | (baud)  // Set baud rate prescaler
#define SPI1_Data8()        SPI1->CTLR1 &= ~(1 << 11)                          // 8-bit data frame (DFF=0)
#define SPI1_Data16()       SPI1->CTLR1 |= (1 << 11)                           // 16-bit data frame (DFF=1)
#define SPI1_MSB()          SPI1->CTLR1 &= ~(1 << 7)                           // MSB first (LSBFIRST=0)
#define SPI1_LSB()          SPI1->CTLR1 |= (1 << 7)                            // LSB first (LSBFIRST=1)
#define SPI1_NSSSw()        SPI1->CTLR1 |= (1 << 9)                            // Software NSS (SSM=1)
#define SPI1_NSSHw()        SPI1->CTLR1 &= ~(1 << 9)                           // Hardware NSS (SSM=0)
#define SPI1_NSSHigh()      SPI1->CTLR1 |= (1 << 8)                            // Set internal NSS high (SSI=1)
#define SPI1_NSSLow()       SPI1->CTLR1 &= ~(1 << 8)                           // Set internal NSS low (SSI=0)
#define SPI1_SSEnable()     SPI1->CTLR2 |= (1 << 2)                            // SS output enable (SSOE=1)
#define SPI1_SSDisable()    SPI1->CTLR2 &= ~(1 << 2)                           // SS output disable (SSOE=0)
#define SPI1_Bidi1()        SPI1->CTLR1 |= (1 << 15)                           // Bidirectional mode (BIDIMODE=1)
#define SPI1_Bidi0()        SPI1->CTLR1 &= ~(1 << 15)                          // Unidirectional mode (BIDIMODE=0)
#define SPI1_TxBidi()       SPI1->CTLR1 |= (1 << 14)                           // TX only in bidi mode (BIDIOE=1)
#define SPI1_RxBidi()       SPI1->CTLR1 &= ~(1 << 14)                          // RX only in bidi mode (BIDIOE=0)
#define SPI1_Master()       SPI1->CTLR1 |= (1 << 2)                            // Master mode (MSTR=1)
#define SPI1_Slave()        SPI1->CTLR1 &= ~(1 << 2)                           // Slave mode (MSTR=0)
#define SPI1_Enable()       SPI1->CTLR1 |= (1 << 6)                            // Enable SPI (SPE=1)
#define SPI1_Disable()      SPI1->CTLR1 &= ~(1 << 6)                           // Disable SPI (SPE=0)

// GPIO Remap for SPI1
// CH32V003: Only remap 0 available (PC5=SCK, PC6=MOSI, PC7=MISO)
// CH32V002/V006: remap 0 = (PC5, PC6, PC7), remap 1 = (PC2, PC1, PC0)
// Note: ch32fun defines GPIO_Remap_SPI1 as a constant, we need function-like macro
#undef GPIO_Remap_SPI1
#ifdef CH32V003
#define GPIO_Remap_SPI1(map)
#else
#define GPIO_Remap_SPI1(map) AFIO->PCFR1 = (AFIO->PCFR1 & ~(1 << 0)) | ((map) << 0)
#endif

#endif //_CH32LIBSDK_H
