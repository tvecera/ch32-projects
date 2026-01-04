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
 * TIM2 Timer Driver for CH32V002/CH32V003
 * ============================================================================
 *
 * Provides TIM2 timer functions for PWM generation and interrupt handling.
 * Used by RCA composite video driver for HSYNC pulse generation.
 *
 * Notes:
 * - Function names are based on CH32LibSDK (https://github.com/Panda381/CH32LibSDK)
 *   for API compatibility
 * - CH32V002 (CH32X00X) uses TIM2_* prefixed register macros
 * - CH32V003 uses generic TIM_* macros
 * - Compatibility macros below unify the API for both MCU variants
 *
 * ============================================================================
 */

#ifndef _TIMER_H
#define _TIMER_H

#include "ch32fun.h"

#ifdef __cplusplus
extern "C" {


#endif

// ============================================================================
// Register Compatibility Macros
// ============================================================================
// CH32X00X (CH32V002) uses TIM2_* prefixed macros, CH32V003 uses generic TIM_*
// Define compatibility macros for unified code
#ifndef TIM_ARPE
#define TIM_ARPE        TIM2_CTLR1_ARPE
#define TIM_CEN         TIM2_CTLR1_CEN
#define TIM_SMS         TIM2_SMCFGR_SMS
#define TIM_OC1M        TIM2_CHCTLR1_OC1M
#define TIM_OC1M_0      TIM2_CHCTLR1_OC1M_0
#define TIM_OC1M_1      TIM2_CHCTLR1_OC1M_1
#define TIM_OC1M_2      TIM2_CHCTLR1_OC1M_2
#define TIM_OC1PE       TIM2_CHCTLR1_OC1PE
#define TIM_OC2M        TIM2_CHCTLR1_OC2M
#define TIM_OC2PE       TIM2_CHCTLR1_OC2PE
#define TIM_CC1E        TIM2_CCER_CC1E
#define TIM_CC2E        TIM2_CCER_CC2E
#define TIM_MOE         TIM1_BDTR_MOE
#define TIM_UG          TIM2_SWEVGR_UG
#define TIM_CC2IF       TIM2_INTFR_CC2IF
#define TIM_CC2IE       TIM2_DMAINTENR_CC2IE

#define TIM_DIR         TIM1_CTLR1_DIR
#define TIM_OC4M        TIM1_CHCTLR1_OC2M
#define TIM_OC4PE       TIM1_CHCTLR2_OC4PE
#define TIM_CC4E        TIM1_CCER_CC4E

#endif

/**
 * Reset TIM2 to default state
 * Performs peripheral reset via RCC
 */
static inline void TIM2_Reset() {
#ifdef CH32V003
    RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
    RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;
#else
    RCC->PB1PRSTR |= RCC_APB1Periph_TIM2;
    RCC->PB1PRSTR &= ~RCC_APB1Periph_TIM2;
#endif
}

/**
 * Enable TIM2 counter
 * Generates update event and starts counting
 */
static inline void TIM2_Enable() {
    TIM2->SWEVGR = TIM_UG;
    TIM2->CTLR1 |= TIM_CEN;
}

/**
 * Disable TIM2 counter
 * Stops counting, preserves configuration
 */
static inline void TIM2_Disable() {
    TIM2->CTLR1 &= ~TIM_CEN;
}

/**
 * Initialize TIM2 with PWM output on specified channel
 * @param chan      Channel number (1 or 2)
 * @param prescaler Timer prescaler (1 = no division)
 * @param period    Timer period (reload value)
 * @param comp      Compare value (pulse width)
 * @param high      Polarity: 1 = active high (PWM mode 1), 0 = active low (PWM mode 2)
 */
static inline void TIM2_InitPWM(const uint8_t chan, const uint16_t prescaler, const uint16_t period,
                                const uint16_t comp, const uint8_t high) {
    RCC->APB1PCENR |= RCC_APB1Periph_TIM2;
    TIM2_Reset();
    TIM2->SMCFGR &= ~TIM_SMS;
    TIM2->PSC = prescaler - 1;
    TIM2->ATRLR = period - 1;
    TIM2->CTLR1 = TIM_ARPE;
#ifdef CH32V003
    TIM2->BDTR |= TIM_MOE;
#endif

    // Configure channel for PWM mode 2 (active low) or mode 1 (active high)
    const uint16_t ocm = high ? (TIM_OC1M_2 | TIM_OC1M_1) : (TIM_OC1M_2 | TIM_OC1M_1 | TIM_OC1M_0);

    if (chan == 1) {
        TIM2->CH1CVR = comp;
        TIM2->CHCTLR1 = (TIM2->CHCTLR1 & ~TIM_OC1M) | ocm | TIM_OC1PE;
        TIM2->CCER |= TIM_CC1E;
    } else {
        TIM2->CH2CVR = comp;
        TIM2->CHCTLR1 = (TIM2->CHCTLR1 & ~TIM_OC2M) | (ocm << 8) | TIM_OC2PE;
        TIM2->CCER |= TIM_CC2E;
    }
    TIM2_Enable();
}

/**
 * Set CH1 compare value
 * @param value Compare value for channel 1
 */
static inline void TIM2_CH1Compare(const uint16_t value) {
    TIM2->CH1CVR = value;
}

/**
 * Set CH2 compare value
 * @param value Compare value for channel 2
 */
static inline void TIM2_Comp2(const uint16_t value) {
    TIM2->CH2CVR = value;
}

/**
 * Enable CH2 output
 * Enables capture/compare output on channel 2
 */
static inline void TIM2_CC2Enable() {
    TIM2->CCER |= TIM_CC2E;
}

/**
 * Disable CH2 output
 * Disables capture/compare output on channel 2
 */
static inline void TIM2_CC2Disable() {
    TIM2->CCER &= ~TIM_CC2E;
}

/**
 * Set CH2 output compare mode
 * @param mode Output compare mode bits (OC2M field value)
 *             Common modes: TIM_COMP_FREEZE, TIM_COMP_ACTIVE, TIM_COMP_INACTIVE,
 *                          TIM_COMP_TOGGLE, TIM_COMP_FORCE_LOW, TIM_COMP_FORCE_HIGH,
 *                          TIM_COMP_PWM1, TIM_COMP_PWM2
 */
static inline void TIM2_OC2Mode(const int mode) {
    TIM2->CHCTLR1 = (TIM2->CHCTLR1 & ~TIM_OC2M) | mode;
}

/**
 * Clear CH2 interrupt flag
 * Clears capture/compare 2 interrupt flag
 */
static inline void TIM2_CC2IntClr() {
    TIM2->INTFR = ~TIM_CC2IF;
}

/**
 * Enable CH2 interrupt
 * Enables capture/compare 2 interrupt
 */
static inline void TIM2_CC2IntEnable() {
    TIM2->DMAINTENR |= TIM_CC2IE;
}

/**
 * Disable CH2 interrupt
 * Disables capture/compare 2 interrupt
 */
static inline void TIM2_CC2IntDisable() {
    TIM2->DMAINTENR &= ~TIM_CC2IE;
}

/**
 * Enable CH2 output compare preload
 * When enabled, compare value is buffered and transferred on update event.
 * Prevents glitches when changing compare value during timer operation.
 */
static inline void TIM2_OC2PreEnable() {
    TIM2->CHCTLR1 |= (1U << 11); // OC2PE bit
}

#ifdef __cplusplus
}
#endif

#endif //_TIMER_H
