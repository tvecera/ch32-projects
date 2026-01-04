// ****************************************************************************
//
//          RVPC - VGA display driver for CH32V002J4M6/CH32V002J4M6
//
// ****************************************************************************
// HSYNC/VSYNC timing targets (reference)
// ****************************************************************************
//
// VMODE 0 and 1 target VGA 640x480@60 Hz timing (@ 50 MHz timer base):
//   Line period T_H = 31.77756 us => 50e6 * T_H = 1600 timer clocks
//   HSYNC pulse = 3.81331 us => ~192 clocks
//   Back porch = 1.90665 us => ~ 96 clocks
//   Active start = 192 + 96 = 288 clocks (matches code)
//   Active span used by bit-bang loop is less than full 1280 clocks to keep margins
//
// VMODE 8 target PAL-like 720x576@~48.02 Hz timing (@ 50 MHz timer base):
//   Text mode: 23 chars * 18 lines (with unroll 5x: 871 cycles active rendering)
//   Line period T_H = 32 us => 50e6 * T_H = 1600 timer clocks
//   HSYNC pulse = 3.81331 us => ~192 clocks
//   Back porch = 1.1 us => ~55 clocks
//   Active start = 192 + 55 = 247 clocks
//   Front porch = ~73 clocks (1.46 us)

// project configuration
#include "config.h"		// project configuration
#include "_config.h"		// default configuration

#if USE_DISP		// 1=use display support

#define	PERIPH_BASE		0x40000000	// Peripheral base address in the alias region
#define APB1PERIPH_BASE		PERIPH_BASE			// 0x40000000
#define APB2PERIPH_BASE		(PERIPH_BASE + 0x10000)		// 0x40010000

#define GPIOA_BASE      (APB2PERIPH_BASE + 0x0800)  // PA port base (0x40010800)
#define GPIOC_BASE		(APB2PERIPH_BASE + 0x1000)	// PC port base (0x40011000)
#define GPIOD_BASE		(APB2PERIPH_BASE + 0x1400)	// PD port 0x40011400
#define TIM1_BASE		(APB2PERIPH_BASE + 0x2C00)	// TIM1 0x40012C00
#define TIM2_BASE		(APB1PERIPH_BASE + 0x0000)	// TIM2 0x40000000

#define DATA_DATAR_OFF		0x0C		// data register offset
#define GPIO_INDR_OFF		0x08		// GPIO input register offset
#define GPIO_BSHR_OFF		0x10		// GPIO bit set output data register
#define GPIO_BCR_OFF		0x14		// GPIO bit clear output data register
#define TIM_INTFR_OFF		0x10		// timer interrupt register offset
#define TIM_CCER_OFF		0x20		// timer compare register
#define TIM_CNT_OFF		    0x24		// timer counter register offset
#define TIM_ATRLR_OFF		0x2C		// ATRLR timer auto-reload value register
#define TIM_CH4CVR_OFF		0x40		// CH4CVR timer compare/capture register 4

#define PS2_DATA_PIN        2           // PA2 - DATA pin
#define PS2_CLK_PIN         1           // PD1 - CLK pin
#define PS2_DATA_MASK       (1 << PS2_DATA_PIN)
#define PS2_CLK_MASK        (1 << PS2_CLK_PIN)

.global FrameBuf			// (u8[]) display graphics buffer
.global DispLine			// (u32) current display line
.global DispFrame			// (u32) current frame
.global FrameBufAddr			// (u8*) current pointer to graphics buffer
.global DispTimTest			// test - get TIM-CNT value at start of image
.global DrawFont			// (u8*) current pointer to font

// If you change the settings, also check this in babypad_key.h.
#define KEYBUF_SIZE	        16
#define KEY_NUM		        8
.global PS2_Data            // (u32[]) PS/2 data buffer
.global PS2_BitCount        // (u8) number of bits in PS/2 data buffer
.global PS2_PrevClk         // (u8) previous state of PS/2 clock
.global KeyState            // (u8) current key state bitmask
.global KeyBuf				// (u8[]) keyboard buffer

.global SoundMelodyPtr		// (sMelodyNote*) pointer to current melody
.global SoundMelodyNext		// (sMelodyNote*) pointer to next melody
.global SoundMelodyLen		// (s16) remaining length of current tone (0 = no melody, -1 = start next melody)

// 32-bit NOP
.macro nopw
	.word	0x13
.endm

// multiply 16-bit NOP
.macro nop_rep num
.rept \num
	c.nop
.endr
.endm

// multiply 32-bit NOP
.macro nopw_rep num
.rept \num
	nopw
.endr
.endm

#if CH32V003 && REDUCE_VGA_SIZE
    .section .text
#else
	.section .time_critical, "ax"
#endif

.macro nopc
#if CH32V003
	c.nop
#endif
.endm

// ============================================================================
// Send byte by bit-banging to GPIOC pin 2 (PC2)
// Each bit is 7 clock cycles
// Total time: 8 bits * 7 clocks = 56 clocks
// ============================================================================
.macro send_byte_bitbang_div8
    andi  a2, a0, 0x80             // [1]
    c.srli  a2, 5                  // [1]
    c.sw    a2, DATA_DATAR_OFF(a5) // [2]
    nop_rep	3                      // [3] -> b7 +7
    andi  a2, a0, 0x40             // [1]
    c.srli  a2, 4                  // [1]
    c.sw    a2, DATA_DATAR_OFF(a5) // [2]
    nop_rep	3                      // [3] -> b6 +14
    andi  a2, a0, 0x20             // [1]
    c.srli  a2, 3                  // [1]
    c.sw    a2, DATA_DATAR_OFF(a5) // [2]
    nop_rep	3                      // [3] -> b5 +21
    andi  a2, a0, 0x10             // [1]
    c.srli  a2, 2                  // [1]
    c.sw    a2, DATA_DATAR_OFF(a5) // [2]
    nop_rep	3                      // [3] -> b4 +28
    andi  a2, a0, 0x08             // [1]
    c.srli  a2, 1                  // [1]
    c.sw    a2, DATA_DATAR_OFF(a5) // [2]
    nop_rep	3                      // [3] -> b3 +35
    andi  a2, a0, 0x04             // [1]
    c.sw    a2, DATA_DATAR_OFF(a5) // [2]
    nop_rep	3                      // [4] -> b2 +42
    andi  a2, a0, 0x02             // [1]
    c.slli  a2, 1                  // [1]
    c.sw    a2, DATA_DATAR_OFF(a5) // [2]
    nop_rep	3                      // [3] -> b1 +49
    andi  a2, a0, 0x01             // [1]
    c.slli  a2, 2                  // [1]
    c.sw    a2, DATA_DATAR_OFF(a5) // [2]
    //nop_rep	3                  // [3] -> b0 +56
.endm

// ============================================================================
// Send 8 pixels from 4KB font buffer (2 pixels per byte)
// Each byte contains 2 pixels in nibbles: high=pixel0, low=pixel1
// Total time: 4 bytes * 8 cycles = 32 cycles
// ============================================================================
.macro send_byte_bitbang_text_mode
    lbu t0, 0(a0)               // [2] Load byte with pixels 0-1
    srli a2, t0, 4              // [1] Extract pixel 0 (high nibble)
    c.sw a2, DATA_DATAR_OFF(a5) // [2] Send pixel 0
    nopc
    andi a2, t0, 0x0f           // [1] Extract pixel 1 (low nibble)
    c.sw a2, DATA_DATAR_OFF(a5) // [2] Send pixel 1 -> +8
    nopc

    lbu t0, 1(a0)               // [2] Load byte with pixels 2-3
    srli a2, t0, 4              // [1] Extract pixel 2
    c.sw a2, DATA_DATAR_OFF(a5) // [2] Send pixel 2
    nopc
    andi a2, t0, 0x0f           // [1] Extract pixel 3
    c.sw a2, DATA_DATAR_OFF(a5) // [2] Send pixel 3 -> +16
    nopc

    lbu t0, 2(a0)               // [2] Load byte with pixels 4-5
    srli a2, t0, 4              // [1] Extract pixel 4
    c.sw a2, DATA_DATAR_OFF(a5) // [2] Send pixel 4
    nopc
    andi a2, t0, 0x0f           // [1] Extract pixel 5
    c.sw a2, DATA_DATAR_OFF(a5) // [2] Send pixel 5 -> +24
    nopc

    lbu t0, 3(a0)               // [2] Load byte with pixels 6-7
    srli a2, t0, 4              // [1] Extract pixel 6
    c.sw a2, DATA_DATAR_OFF(a5) // [2] Send pixel 6
    nopc
    andi a2, t0, 0x0f           // [1] Extract pixel 7
    c.sw a2, DATA_DATAR_OFF(a5) // [2] Send pixel 7 -> +32
.endm

// ============================================================================
// VMODE 0: graphics mode 128x64 pixels
//          target VGA 640x480@60 Hz
//          required memory 1024 B
//
// 16 samples per line, 1 sample = 1 src byte = 8x bit banged pixels)
// Using uniform 7 cycles/bit (send_byte_bitbang_div8 = 56 cycles)
// ITERATION subtotal (without branch) = 2+1+56+1 = 60 cycles
// Active payload = 16 * 60 + 46 + 2 = 960 + 48 = 1008 cycles
// 1280 - 1008 = 272 cycles margin inside visible 1280 clocks.
// ============================================================================

#if VMODE == 0

// Registers:
//  A0-A3, T0-T1 =
//  A4 = pointer to source graphics buffer
//  A5 = GPIOC base
//  T2 = current scanline
//  RA = return address
// - continue to TIM2_IRQHandler4

.macro vga_draw

	// test - get TIM-CNT value at start of image (should be 288-5=283)
	//li	a0,TIM2_BASE		// A0 <- Timer 2 base
	//lw	a0,TIM_CNT_OFF(a0)	// load timer counter
	//la	a1,DispTimTest
	//sw	a0,0(a1)		// save value

	// prepare number of samples -> A3
	li	a3,16			// -5: [1]

	.align	2,,			// -4: [1]

// [64 loop] send sample ... This address must be aligned

// Registers:
//  A0 = temporary with sample
//  A1, T0-T1 =
//  A2 = delay counter
//  A3 = loop counter
//  A4 = pointer to source graphics buffer
//  A5 = GPIOC base
//  T2 = current scanline
//  RA = return address

	// [5] send sample
2:	lbu	a0,0(a4)		    // -3: [2] A0 <- load sample
	addi	a4,a4,1			// -1: [1] increase sample address
	send_byte_bitbang_div8	// 0: [56] send sample

	// [2,4] loop counter
	c.addi	a3,-1		    // +56: [1]
	c.bnez	a3,2b			// +57: [1,3]
					        // +58 (pass), +60 (loop)

	// send black color
	sw	x0,DATA_DATAR_OFF(a5)	// send black sample

	// save new sample pointer only every 4th scanline (divide line by 4)
	li	a1,3			// to compare
	and	a0,t2,a1		// get lowest 2 bits
	bne	a0,a1,1f		// skip if not correct scanline
	la	a2,FrameBufAddr
	sw	a4,0(a2)		// save new pointer

1:	la	a0,TIM2_IRQHandler4
	jr	a0

.endm

#endif // VMODE == 0

// ============================================================================
// !!! ONLY FOR CH32V002 !!!
//
// VMODE 1: graphics mode 160x120 pixels
//          target VGA 640x480@60 Hz
//          required memory 2400 B
//
// 20 samples per line, 1 sample = 1 src byte = 8x bit banged pixels)
// Using uniform 7 cycles/bit (send_byte_bitbang_div8 = 56 cycles)
// ITERATION subtotal (without branch) = 2+1+56+1 = 60 cycles
// Active payload = 20 * 60 + 58 + 2 = 1200 + 60 = 1260 cycles
// 1280 * 1260 = 20 cycles margin inside visible 1280 clocks.

#if VMODE == 1

// Registers:
//  A0-A3, T0-T1 =
//  A4 = pointer to source graphics buffer
//  A5 = GPIOC base
//  T2 = current scanline
//  RA = return address
// - continue to TIM2_IRQHandler4

.macro vga_draw

	// test - get TIM-CNT value at start of image (should be 288-5=283)
	//li	a0,TIM2_BASE		// A0 <- Timer 2 base
	//lw	a0,TIM_CNT_OFF(a0)	// load timer counter
	//la	a1,DispTimTest
	//sw	a0,0(a1)		// save value

	// prepare number of samples -> A3
	li	a3,20			// -5: [1]

	.align	2,,			// -4: [1]

// [64 loop] send sample ... This address must be aligned

// Registers:
//  A0 = temporary with sample
//  A1, T0-T1 =
//  A2 = delay counter
//  A3 = loop counter
//  A4 = pointer to source graphics buffer
//  A5 = GPIOC base
//  T2 = current scanline
//  RA = return address

	// [5] send sample
2:	lbu	a0,0(a4)		    // -3: [2] A0 <- load sample
	addi	a4,a4,1			// -1: [1] increase sample address
	send_byte_bitbang_div8	// 0: [56] send sample

	// [2,4] loop counter
	c.addi	a3,-1		    // +56: [1]
	c.bnez	a3,2b			// +57: [1,3]
					        // +58 (pass), +60 (loop)

	// send black color
	sw	x0,DATA_DATAR_OFF(a5)	// send black sample

	// save new sample pointer only every 4th scanline (divide line by 4)
	li	a1,3			// to compare
	and	a0,t2,a1		// get lowest 2 bits
	bne	a0,a1,1f		// skip if not correct scanline
	la	a2,FrameBufAddr
	sw	a4,0(a2)		// save new pointer

1:	la	a0,TIM2_IRQHandler4
	jr	a0

.endm

#endif // VMODE == 1


// ============================================================================
// VMODE 8: text mode 23x18 characters
//          target "PAL-like" 720x576@~48.02 Hz
//          required memory 414 B (23 chars * 18 lines)
//
// Font format: 4KB (2 pixels per byte)
// Loop unrolling: 5 iterations of 4 characters + 3 final characters
//
// TIMING OPTIMIZATION:
// Per 4-char block: 4 chars * 37 cycles + loop overhead 4 = 152 cycles
// 5 blocks: 5 * 152 = 760 cycles
// Final 3 chars: 3 * 37 = 111 cycles
// Total: ~871 cycles
// ============================================================================

#if VMODE == 8

.macro vga_draw

    // prepare pointer to font -> A1
    la a1,DrawFont       // [2] address of pointer
    lw a1,0(a1)          // [2] A1 <- get pointer to font

    // Shift font to current font line -> A1
    // For 4KB font: each row is 128 chars * 4 bytes = 512 bytes per row
    andi a0,t2,0x1C     // [1] A0 <- mask bits [4:2] (0x1C = 0b11100)
#if FONT_CHAR_COUNT == 64
    slli a0,a0,6       // [1] A0 <- shift left by 6
#else
    slli a0,a0,7        // [1] A0 <- shift left by 7
#endif
    add    a1,a1,a0      // [1] A1 <- shift pointer to current font line

    // Prepare loop counter for 5 iterations (4 chars each)
    li     a3,5          // [1] 5 iterations of 4 characters

    .align 2

    // ===== Main loop: process 4 characters per iteration =====
2:
    // Character 1 of 4
    lbu    a2,0(a4)      // [2] load character
    add    a4,a4,1       // [1] increment pointer
    slli   a0,a2,2       // [1] index * 4
    add    a0,a0,a1      // [1] font address
    send_byte_bitbang_text_mode // [32] send char 1

    // Character 2 of 4
    lbu    a2,0(a4)      // [2]
    add    a4,a4,1       // [1]
    slli   a0,a2,2       // [1]
    add    a0,a0,a1      // [1]
    send_byte_bitbang_text_mode // [32] send char 2

    // Character 3 of 4
    lbu    a2,0(a4)      // [2]
    add    a4,a4,1       // [1]
    slli   a0,a2,2       // [1]
    add    a0,a0,a1      // [1]
    send_byte_bitbang_text_mode // [32] send char 3

    // Character 4 of 4
    lbu    a2,0(a4)      // [2]
    add    a4,a4,1       // [1]
    slli   a0,a2,2       // [1]
    add    a0,a0,a1      // [1]
    send_byte_bitbang_text_mode // [32] send char 4

    // Loop control
    add    a3,a3,-1      // [1] decrement counter
    bnez   a3,2b         // [3] loop if more blocks remain
    // Total per iteration: 4*37 + 4 = 152 cycles

    // ===== Final 3 characters (21, 22, 23) =====
    lbu    a2,0(a4)      // [2] char 21
    add    a4,a4,1       // [1]
    slli   a0,a2,2       // [1]
    add    a0,a0,a1      // [1]
    send_byte_bitbang_text_mode // [32]

    lbu    a2,0(a4)      // [2] char 22
    add    a4,a4,1       // [1]
    slli   a0,a2,2       // [1]
    add    a0,a0,a1      // [1]
    send_byte_bitbang_text_mode // [32]

    lbu    a2,0(a4)      // [2] char 23
    add    a4,a4,1       // [1]
    slli   a0,a2,2       // [1]
    add    a0,a0,a1      // [1]
    send_byte_bitbang_text_mode // [32]

    // send black color
    sw	x0,DATA_DATAR_OFF(a5)	// send black sample

    // Epilog - save pointer every 32nd scanline
    andi   a0,t2,0x1f    // [1] mask lowest 5 bits
    xori   a1,a0,0x1f    // [1] check if == 0x1f
    bnez   a1,1f         // [3] skip if not 31
    la     a2,FrameBufAddr // [2] load address
    sw     a4,0(a2)      // [2] save new pointer

1:  la a0,TIM2_IRQHandler4
    jr a0
.endm

#endif // VMODE == 8

// ============================================================================
//                   Timer handler - draw VGA graphics line
// ============================================================================
// Hardware prologie/epilogue HPE (2 levels):
//  Interrupt saves working registers: x1 (ra), x5-x7 (t0-t2), x10-x15 (a0-a5)

	.align  4,,
.global TIM2_IRQHandler
TIM2_IRQHandler:

// ==== Clear interrupt flag
// Registers:
//  A0-A5, T0-T2 =
//  RA = return address

	// clear update interrupt flag (Timer 2, channel 2)
	li	a2,TIM2_BASE		// A2 <- Timer 2 base
	lw	a1,TIM_INTFR_OFF(a2)	// get INTFR register
	lw	a0,TIM_CNT_OFF(a2)	// A0 <- load timer counter
	andi	a1,a1,~(1<<2)		// clear interrupt flag
	sw	a1,TIM_INTFR_OFF(a2)	// set INTFR register

// ==== Time synchronization
// Registers:
//  A0 = Timer 2 counter value
//  A1-A5,T0-T2 =
//  RA = return address

	//  The exact interruption time may vary within a range of 1-7 processor clock
	//  cycles due to the varying length of instructions in the main program.

	// >>>>>>>> SETUP
	//  If you change Timer 2 channel 2 compare value, setup this
	//  correction - select 0 to 7 to minimise display noise.
#ifdef VMODE_SHIFT	// shift VMODE time correction
	addi	a0,a0,VMODE_SHIFT	// time correction: select 0 to 7
#else

#if VMODE == 0
	addi	a0,a0,7			// time correction: select 0 to 7
#elif VMODE == 1
	addi	a0,a0,7			// time correction: select 0 to 7
#elif VMODE == 8
#if CH32V003
	addi	a0,a0,2		// time correction: select 0 to 7
#else
    addi	a0,a0,0
#endif
#endif

#endif
	// <<<<<<<<

	andi	a0,a0,7			// mask lower 3 bits of the timer counter
	slli	a0,a0,1			// timer * 2 (because jump to NOPs is multiply of 2 bytes)
	la	a2,TIM2_IRQHandler_sync	// table of 7x NOP instruction
	add	a0,a0,a2		// A0 <- jump address

	.align  2,,
	jr	a0			// jump to NOP and delay additional 'x' clock cycles
	.align	2,,

TIM2_IRQHandler_sync:
	nop_rep	7			// 0..7: +7..+0 clk

// ==== Get current scanline

	// load current scanline -> T2
	la	a0,DispLine
	lw	t2,0(a0)

// Registers:
//  A0-A5, T0-T1 =
//  T2 = current scanline
//  RA = return address

	// check active image
#if VMODE == 0
	li	a0,256
#elif VMODE == 8
	li	a0,576
#else
	li	a0,480
#endif
	bge	t2,a0,TIM2_IRQHandler2 // not active image

// ==== active image

	// load pointer to graphics buffer -> A4
	la	a4,FrameBufAddr
	lw	a4,0(a4)

	// pointer to GPIOC base -> A5
	li	a5,GPIOC_BASE

// Registers:
//  A0-A2, T0-T1 =
//  A3 = pointer to attribute buffer
//  A4 = pointer to source graphics buffer
//  A5 = GPIOC base
//  T2 = current line
//  RA = return address

	// draw VGA line (continue to TIM2_IRQHandler4)
	vga_draw


// ==== VSYNC
// Registers:
//  A0-A5, T0-T1 =
//  T2 = current scanline
//  RA = return address

	.align  2,,

TIM2_IRQHandler2:

	la	a0,TIM2_IRQHandler2B
	jr	a0
	.section .text
TIM2_IRQHandler2B:

	// check front porch
#if VMODE == 0
	li	a1,256+112+10
#elif VMODE == 8
	li	a1,576+39
#else
	li	a1,480+10
#endif
	blt	t2,a1,TIM2_IRQHandler4	// front porch - black line

	// VSYNC
	li	a3,GPIOD_BASE		    // A3 <- port D
    li	a0,1<<6                 // A0 <- PD6 mask

// Registers:
//  A1-A2,A4,A5, T0-T1 =
//  A0 = mask of pin 6
//  A3 = GPIOD base
//  T2 = current scanline
//  RA = return address

#if VMODE == 0
	li	a1,256+112+10+2 - 1	// A1 <- index of last VSYNC scanline
#elif VMODE == 8
    li	a1,576+39+2 - 1 	// A1 <- index of last VSYNC scanline
#else
	li	a1,480+10+2 - 1		// A1 <- index of last VSYNC scanline
#endif
	bgt	t2,a1,TIM2_IRQHandler3	// not VSYNC, serve back porch

	// start VSYNC pulse (send 0 to PD6)
	sw	a0,GPIO_BCR_OFF(a3)	// send 0 to PD6
	bne	t2,a1,TIM2_IRQHandler4	// skip if not last line of VSYNC

// ==== Music
// Registers:
//  A0-A5, T0-T1 =
//  T2 = current scanline
//  RA = return address

#if USE_SOUND && !SOUND_MUTE		// 1=use sound support

// Registers:
//  A0 = remaining length
//  A1 = SoundMelodyLen variable
//  A2 = SoundMelodyPtr variable
//  A3 = melody pointer
//  A4 = TIM1 base
//  A5,T0-T1 =
//  T2 = current scanline
//  RA = return address

	// get tone counter
	la	a1,SoundMelodyLen
	lh	a0,0(a1)		// A0 <- remaining length
	beqz	a0,TIM2_IRQHandler4	// no melody, quit
	la	a2,SoundMelodyPtr
	li	a4,TIM1_BASE		// A4 <- Timer 1 base
	bgtz	a0,4f			// valid counter

// The audio output is via PC4 (pin 4), Timer 1 channel 4, mapping 0.

	// request to start new melody
	la	a3,SoundMelodyNext
	lw	a3,0(a3)		// A3 <- pointer to next melody
	sw	a3,0(a2)		// save new pointer
	j	5f			// start new note

	// decrease counter
4:	addi	a0,a0,-1		// shift length counter
	sh	a0,0(a1)		// save new counter
	bnez	a0,TIM2_IRQHandler4	// counter not 0, continue with this tone

	// shift current melody pointer
	lw	a3,0(a2)		// A3 <- current melody pointer
	addi	a3,a3,4			// shift melody pointer
	sw	a3,0(a2)		// save new pointer

	// start new note
5:	lhu	a0,0(a3)		// A0 <- note length
	sh	a0,0(a1)		// save counter
	beqz	a0,8f			// stop melody

	// get note divider
	lhu	a3,2(a3)		// A3 <- note divider
	beqz	a3,8f			// pause

	// set timer load
	sw	a3,TIM_ATRLR_OFF(a4)	// save auto-reload register
	srli	a3,a3,1			// divider / 2
	sw	a3,TIM_CH4CVR_OFF(a4)	// save compare register

	// enable melody
	lw	a0,TIM_CCER_OFF(a4)	// load compare register
	li	a1,1<<12
	or	a0,a0,a1		// enable channel 4
	sw	a0,TIM_CCER_OFF(a4)	// save compare register
	j	TIM2_IRQHandler4

	// stop melody
8:	lw	a0,TIM_CCER_OFF(a4)	// load compare register
	li	a1,~(1<<12)
	and	a0,a0,a1		// disable channel 4
	sw	a0,TIM_CCER_OFF(a4)	// save compare register

#endif // USE_SOUND

	j	TIM2_IRQHandler4

// ==== Back porch
// Registers:
//  A1-A2,A4,A5,T0-T1 =
//  A0 = mask of pin 7
//  A3 = GPIOC base
//  T2 = current scanline
//  RA = return address

TIM2_IRQHandler3:

	// back porch - stop VSYNC pulse (1 to PD6)
	sw	a0,GPIO_BSHR_OFF(a3)

#if USE_KEYS    // 1=use keyboard support

	la	a2, PS2_BitCount
	lbu	t0, 0(a2)
	c.li	a1, 11
	bltu	t0, a1, TIM2_IRQHandler4

ps2_process_loop:
	// Load PS2_Data[0] and PS2_Data[1]
	la	a5, PS2_Data
	lw	t1, 0(a5)         // t1 = PS2_Data[0] (low)
	lw	a4, 4(a5)         // a4 = PS2_Data[1] (high)

	// Validate first frame
	andi	a3, t1, 0x401     // Extract bits 0 and 10
	xori	a3, a3, 0x400     // Should be 0x400 (bit 10 set, bit 0 clear)
	bnez	a3, ps2_error     // Invalid frame

	// Extract first byte
	srli	a1, t1, 1
	andi	a1, a1, 0xFF

	// Decode prefix and jump to handler
	xori	a3, a1, 0xF0      // Check for break prefix
	beqz	a3, ps2_prefix_break
	xori	a3, a1, 0xE0      // Check for extended prefix
	beqz	a3, ps2_prefix_extended

	j	ps2_map_make_single

ps2_prefix_break:
	c.li	a3, 22
	bltu	t0, a3, TIM2_IRQHandler4

	srli	a0, t1, 11
	andi	a3, a0, 0x401
	xori	a3, a3, 0x400
	bnez	a3, ps2_error

	srli	a0, a0, 1
	andi	a0, a0, 0xFF
	slli	a1, a1, 8
	or	a1, a1, a0

	j	ps2_map_break_normal

ps2_prefix_extended:
	c.li	a3, 22
	blt	t0, a3, TIM2_IRQHandler4

	srli	a0, t1, 11
	andi	a3, a0, 0x401
	xori	a3, a3, 0x400
	bnez	a3, ps2_error

	srli	a0, a0, 1
	andi	a0, a0, 0xFF
	xori	a3, a0, 0xF0
	beqz	a3, ps2_prefix_extended_break

	slli	a1, a1, 8
	or	a1, a1, a0
	j	ps2_map_make_extended

ps2_prefix_extended_break:
	li	a3, 33
	blt	t0, a3, TIM2_IRQHandler4

	// Validate third frame (bits 22-32)
	srli	a0, t1, 22        // Start bit (bit 22)
	andi	a3, a0, 1
	c.bnez	a3, ps2_error

	// Check stop bit (bit 32 = bit 0 of PS2_Data[1])
	andi	a3, a4, 1
	c.beqz	a3, ps2_error

	// Extract scancode (bits 23-30)
	srli	a0, t1, 23
	andi	a0, a0, 0xFF

	li	a1, 0xE1
	slli	a1, a1, 8
	or	a1, a1, a0

	j	ps2_map_break_extended

ps2_map_make_single:
	la	a3, KeyState
	lbu	a4, 0(a3)
	c.li	a2, 11

    li a0, 0x1C
    beq a1, a0, 2f
    li a0, 0x29
    beq a1, a0, 2f
    li a0, 0x5A
    bne a1, a0, 1f
2:	ori	a4, a4, (1<<6)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	li	a0, 0x32
	bne	a1, a0, 1f
	ori	a4, a4, (1<<7)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	li	a0, 0x22
	bne	a1, a0, 1f
	ori	a4, a4, (1<<4)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	li	a0, 0x35
	bne	a1, a0, 1f
	ori	a4, a4, (1<<5)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	j	ps2_shift_and_store

ps2_map_make_extended:
	la	a3, KeyState
	lbu	a4, 0(a3)
	c.li	a2, 22

	li	t0, 0x6B
	bne	a0, t0, 1f
	ori	a4, a4, (1<<2)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	li	t0, 0x74
	bne	a0, t0, 1f
	ori	a4, a4, (1<<0)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	li	t0, 0x75
	bne	a0, t0, 1f
	ori	a4, a4, (1<<1)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	li	t0, 0x72
	bne	a0, t0, 1f
	ori	a4, a4, (1<<3)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	j	ps2_shift_and_store

ps2_map_break_normal:
	la	a3, KeyState
	lbu	a4, 0(a3)
	c.li	a2, 22

    li t0, 0x1C
    beq a0, t0, 2f
    li t0, 0x29
    beq a0, t0, 2f
    li t0, 0x5A
    bne a0, t0, 1f
2:  andi a4, a4, ~(1<<6)
    sb a4, 0(a3)
    j ps2_shift_and_store

1:	li	t0, 0x32
	bne	a0, t0, 1f
	andi	a4, a4, ~(1<<7)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	li	t0, 0x22
	bne	a0, t0, 1f
	andi	a4, a4, ~(1<<4)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	li	t0, 0x35
	bne	a0, t0, 1f
	andi	a4, a4, ~(1<<5)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	j	ps2_shift_and_store

ps2_map_break_extended:
	la	a3, KeyState
	lbu	a4, 0(a3)
	li	a2, 33

	li	t0, 0x6B
	bne	a0, t0, 1f
	andi	a4, a4, ~(1<<2)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	li	t0, 0x74
	bne	a0, t0, 1f
	andi	a4, a4, ~(1<<0)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	li	t0, 0x75
	bne	a0, t0, 1f
	andi	a4, a4, ~(1<<1)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	li	t0, 0x72
	bne	a0, t0, 1f
	andi	a4, a4, ~(1<<3)
	sb	a4, 0(a3)
	j	ps2_shift_and_store

1:	j	ps2_shift_and_store

ps2_shift_and_store:
	// Store to KeyBuf (circular buffer)
	la	a3, KeyWriteOff
	lbu	a0, 0(a3)
	addi	a4, a0, 1
	li	a5, KEYBUF_SIZE
	bltu	a4, a5, 2f
	mv	a4, x0
2:	la	a5, KeyReadOff
	lbu	a5, 0(a5)
	beq	a4, a5, 3f

	la	a5, KeyBuf
	slli	a0, a0, 1
	add	a5, a5, a0
	sh	a1, 0(a5)
	sb	a4, 0(a3)

3:	// Shift PS2_Data
	la	a5, PS2_Data
	lw	t1, 0(a5)
	lw	a3, 4(a5)

	srl	t1, t1, a2
	li	a4, 32
	sub	a4, a4, a2
	sll	a0, a3, a4
	or	t1, t1, a0
	sw	t1, 0(a5)

	srl	a3, a3, a2
	sw	a3, 4(a5)

	la	a0, PS2_BitCount
	lbu	t0, 0(a0)
	sub	t0, t0, a2

	c.li	a1, 11
	bge	t0, a1, 4f
	mv	t0, x0
	la	a5, PS2_Data
	sw	x0, 0(a5)
	sw	x0, 4(a5)

4:	sb	t0, 0(a0)        // Uložit (uprávený) BitCount

	c.li	a1, 11
	bge	t0, a1, ps2_process_loop
	j	TIM2_IRQHandler4

ps2_error:
	la	a5, PS2_Data
	sw	x0, 0(a5)     // Clear PS2_Data[0]
	sw	x0, 4(a5)     // Clear PS2_Data[1]
	la	a2, PS2_BitCount
	sb	x0, 0(a2)
	la	a3, PS2_PrevClk
	sb	x0, 0(a3)
    la	a3, KeyState
    sb	x0, 0(a3)     // Clear all key states

#endif // USE_KEYS

//	j	TIM2_IRQHandler4

// ==== Increase scanline
// Registers:
//  A0-A5, T0-T1 =
//  T2 = current scanline
//  RA = return address

TIM2_IRQHandler4:

	// inrease current scanline
	addi	t2,t2,1			// T2 <- next scanline
#if VMODE == 8
	li	a2,625			// A2 <- total number of scanlines
#else
	li	a2,525			// A2 <- total number of scanlines
#endif
	blt	t2,a2,8f		// not total line yet

	// reset to start of image
	li	t2,0			// T2 <- new scanline = 0

	// increase current frame
	la	a0,DispFrame		// A0 <- current frame
	lw	a1,0(a0)		// A1 <- get current frame
	addi	a1,a1,1			// increase current frame
	sw	a1,0(a0)		// save new current frame

	// reset pointers
	la	a0,FrameBuf		// A0 <- frame buffer
	la	a1,FrameBufAddr		// A1 <- frame buffer address
	sw	a0,0(a1)		// save new pointer

	// save new scanline
8:	la	a0,DispLine
	sw	t2,0(a0)		// save new scanline

#if USE_KEYS
    li   a3, GPIOD_BASE
    lw   a0, GPIO_INDR_OFF(a3)
    la   a1, PS2_PrevClk
    lbu  a2, 0(a1)
    c.srli a0, PS2_CLK_PIN
    andi a0, a0, 1

    c.bnez a0, ps2_save_clk
    c.beqz a2, ps2_save_clk

    // FALLING EDGE! Read DATA
    li   a3, GPIOA_BASE
    lw   a3, GPIO_INDR_OFF(a3)
    c.srli a3, PS2_DATA_PIN
    c.andi a3, 1

    // Add bit to PS2_Data[2]
    la   t0, PS2_BitCount
    lbu  t1, 0(t0)
    la   a5, PS2_Data             // a5 = base address of array

    // Check if bit position < 32
    li   a4, 32
    blt  t1, a4, ps2_store_low

    // Store to PS2_Data[1] (high 32 bits)
    lw   a4, 4(a5)                // Load PS2_Data[1]
    addi t1, t1, -32
    sll  a3, a3, t1
    or   a4, a4, a3
    sw   a4, 4(a5)                // Store to PS2_Data[1]

    la   t0, PS2_BitCount
    lbu  t1, 0(t0)
    c.addi t1, 1
    sb   t1, 0(t0)
    j    ps2_save_clk

ps2_store_low:
    // Store to PS2_Data[0] (low 32 bits)
    lw   a4, 0(a5)                // Load PS2_Data[0]
    sll  a3, a3, t1
    or   a4, a4, a3
    sw   a4, 0(a5)                // Store to PS2_Data[0]

    c.addi t1, 1
    sb   t1, 0(t0)

ps2_save_clk:
    sb   a0, 0(a1)

#endif // USE_KEYS
	mret

#endif // USE_DISP