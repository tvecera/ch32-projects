
// ****************************************************************************
//
//                        TinyBoy - Device init
//
// ****************************************************************************

#include INCLUDES_H     // all includes
#include "_include.h"

#if USE_RCA
INLINE void DispInit() { rca_init(); }

INLINE void DispTerm() { rca_stop(); }

INLINE void DispUpdate() { rca_wait_vsync(); }

// Frame end callback - in RAM for fast execution
// NOTE: Only SoundScan here! KeyScan is too slow (ADC blocking).
void frame_callback() {
	KeyScan();
#if USE_SOUND > 1
	SoundScan();
#endif
}

// RCA mode: I2C emulation for framebuffer writes
static u8 RCA_DispOutPage = 0;  // current page (0..7)
static u8 RCA_DispOutX = 0;     // current X position (0..127)

// Select page (0-7) and reset X position
void DispI2C_SelectPage(int page) {
	RCA_DispOutPage = page & 7;
	RCA_DispOutX = 0;
}

// Write byte to framebuffer (converts SSD1306 vertical format to horizontal)
void DispI2C_Write(u8 data) {
	int y = RCA_DispOutPage;
	int x = RCA_DispOutX;

	if ((x < 128) && (y < 8)) {
		u8* d = &FrameBuf[(x >> 3) + y * (WIDTHBYTE * 8)];
		u8 mask = 1 << (7 - (x & 7));

		// Convert vertical byte to horizontal format
		if ((data & B0) == 0) d[0 * WIDTHBYTE] &= ~mask; else d[0 * WIDTHBYTE] |= mask;
		if ((data & B1) == 0) d[1 * WIDTHBYTE] &= ~mask; else d[1 * WIDTHBYTE] |= mask;
		if ((data & B2) == 0) d[2 * WIDTHBYTE] &= ~mask; else d[2 * WIDTHBYTE] |= mask;
		if ((data & B3) == 0) d[3 * WIDTHBYTE] &= ~mask; else d[3 * WIDTHBYTE] |= mask;
		if ((data & B4) == 0) d[4 * WIDTHBYTE] &= ~mask; else d[4 * WIDTHBYTE] |= mask;
		if ((data & B5) == 0) d[5 * WIDTHBYTE] &= ~mask; else d[5 * WIDTHBYTE] |= mask;
		if ((data & B6) == 0) d[6 * WIDTHBYTE] &= ~mask; else d[6 * WIDTHBYTE] |= mask;
		if ((data & B7) == 0) d[7 * WIDTHBYTE] &= ~mask; else d[7 * WIDTHBYTE] |= mask;

		x++;
		if (x >= 128) {
			x = 0;
			RCA_DispOutPage++;
		}
		RCA_DispOutX = x;
	}
}

// Stop I2C communication (no-op for RCA mode)
void DispI2C_Stop(void) {
	// Nothing to do - framebuffer writes are immediate
}

#else

u32 SysTick_OldCnt2;

// SysTick handler
__attribute__((interrupt)) void SysTick_Handler()
{
	// clear interrupt request
	SysTick_ClrCmp();

	// get new counter
	u32 cnt = SysTick_Get();

	// load current time
	u32 sys = SysTime;
	u16 ms = CurTimeMs;
	u32 old = SysTick_OldCnt2;

	// increase time
	u32 dif = cnt - old;
	while (dif >= 384000)
	{
		dif -= 384000;
		old += 384000;
		sys += 16;
		ms += 16;
		if (ms >= 1000)
		{
			UnixTime++;
			ms -= 1000;
		}
	}

	// save new current time
	SysTime = sys;
	CurTimeMs = ms;
	SysTick_OldCnt2 = old;

	// set new interrupt time (skip one interrupt, if remains < 100us)
	cnt += 384000 - dif;
	if (dif >= 384000 - 100*HCLK_PER_US) cnt += 384000;
	SysTick_SetCmp(cnt);

	KeyScan();
#if USE_SOUND > 1
	SoundScan();
#endif
}

#endif

// Initialize device
void DevInit()
{
#if USE_KEYS		// 1=use keyboard support
	// Initialize keyboard service (should be called before display init)
	KeyInit();
#endif

#if USE_SOUND		// use sound support 1=tone, 2=melody
	// Sound initialize
	SoundInit();
#endif

#if USE_DISP		// 1=use display support
	// Display initialize
	DispInit();
#endif

#if USE_DRAW || USE_PRINT
	// clear screen
	DrawClear();
#endif

#if USE_RCA
	rca_set_frame_callback(frame_callback);
#else
	NVIC_IRQEnable(IRQ_SYSTICK); // IRQ enable
	SysTick_IntEnable();	// interrupt enable
#endif
}

// Terminate device
void DevTerm()
{
#if USE_KEYS		// 1=use keyboard support
	// wait for no key pressed
	KeyWaitNoPressed();
#endif

#if USE_DISP		// 1=use display support
	// Display terminate
	DispTerm();
#endif

#if USE_SOUND		// use sound support 1=tone, 2=melody
	// Sound terminate
	SoundTerm();
#endif

#if USE_KEYS		// 1=use keyboard support
	// terminate keyboard
	KeyTerm();
#endif
}
