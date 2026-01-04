// ****************************************************************************
//
//                        RVPC - VGA display driver for CH32V002J4M6
//
// ****************************************************************************
#include INCLUDES_H     // all includes
#include "_include.h"

#if USE_DISP	// 1=use display support

#if !USE_RCA

u8 FrameBuf[FRAMESIZE];     // display graphics buffer
volatile u32 DispLine;      // current display line
volatile u32 DispFrame = 0; // current frame
volatile u8* FrameBufAddr;  // current pointer to graphics buffer
volatile u32 DispTimTest;   // test - get TIM-CNT value at start of image

// wait for VSync scanline
void WaitVSync() {
    while (DispIsVSync()) {
    }
    while (!DispIsVSync()) {
    }
}

// Initialize videomode
void DispInit() {
    DispLine = 0; // current display line
    FrameBufAddr = FrameBuf; // current pointer to graphics buffer

    // trim HSI oscillator to 25MHz
    RCC_HSITrim(31);

    // initialize GPIO pins (note: clocks are setup at config.h)
    // - PC2 video out, GPIO push-pull output, fast speed
    RCC_AFIClkEnable();
    RCC_PCClkEnable();
    RCC_PDClkEnable();

    // PC2 - VSYNC, GPIO push-pull output, fast speed
    GPIO_Mode(PD6, GPIO_MODE_OUT_FAST);
    // PC1 - HSYNC, Timer 2 Channel 1 output (remap 2)
    GPIO_Mode(PC1, GPIO_MODE_AF_FAST);

    // PC1 - VIDEO output, GPIO push-pull output, fast speed
    GPIO_Mode(PC2, GPIO_MODE_OUT_FAST);

// Remap Timer 2 (output channel 1 to PC1)
#if CH32V003
GPIO_Remap_TIM2(2);
#else
GPIO_Remap_TIM2(1);
#endif

// -----------------------------------------------------------------------------
// HSYNC/VSYNC timing targets (reference)
// -----------------------------------------------------------------------------
// VMODE 0 and 1 target VGA 640x480@60 Hz timing (@ 50 MHz timer base):
// Line period T_H = 31.77756 us => 50e6 * T_H ~ 1600 timer clocks
// HSYNC pulse ~3.81331 us => ~192 clocks
// Back porch ~1.90665 us => ~96 clocks
// Active start = 192 + 96 = 288 clocks
//
// VMODE 8 target PAL-like 720x576@~48.02 Hz timing (@ 50 MHz timer base):
// Text mode: 23 chars * 18 lines (with unroll 5x: 871 cycles active rendering)
// Line period T_H = 32 us => 50e6 * T_H = 1600 timer clocks
// HSYNC pulse ~3.81331 us => ~192 clocks
// Back porch ~1.1 us => ~55 clocks
// Active start = 192 + 55 = 247 clocks
// Front porch = ~73 clocks (1.46 us)

TIM2_InitPWM(1, 1, 1600 - 1, 192, False);

// Setup interrupt on Timer 2 channel 1 to display image
//  128x64: 8 clock cycles per pixel, 64 clock cycles per character, active 16 characters, total 25 characters
//  160x120: 8 clock cycles per pixel, 64 clock cycles per character, active 20 characters, total 25 characters
#if VMODE == 0
TIM2_Comp2(288 - 60); // set compare value
#elif VMODE == 1
TIM2_Comp2(288 - 60); // set compare value
#elif VMODE == 8
TIM2_Comp2(247 + 5); // set compare value
#endif
TIM2_OC2Mode(TIM_COMP_FREEZE); // set compare mode
TIM2_OC2PreEnable(); // enable preload compare register
TIM2_CC2Enable(); // enable compare output
TIM2_CC2IntClr(); // clear interrupt request
TIM2_CC2IntEnable(); // enable capture compare of channel 2
NVIC_IRQEnable(IRQ_TIM2); // enable interrupt service
}

// Terminate videomode
void DispTerm() {
    // disable capture compare interrupt (channel 2)
    NVIC_IRQDisable(IRQ_TIM2); // disable interrupt service
    TIM2_CC2IntDisable(); // disable capture compare of channel 2
    TIM2_CC2Disable(); // disable compare output
    TIM2_CC2IntClr(); // enable capture compare of channel 2

    // disable timer
    TIM2_Disable();

    // Reset timer to default setup
    TIM2_Reset();

    // reset GPIO pins
    GPIO_PinReset(PD6);
    GPIO_PinReset(PC1);
    GPIO_PinReset(PC2);

    // reset HSI trim to default value 24 MHz
    RCC_HSITrim(16);
}

#endif // USE_RCA
#endif // USE_DISP
