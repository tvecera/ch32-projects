
// ****************************************************************************
//
//                        TinyBoy - Device init
//
// ****************************************************************************

#include INCLUDES_H     // all includes
#include "_include.h"

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
#if USE_SOUND
	SoundScan();
#endif
}

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

	NVIC_IRQEnable(IRQ_SYSTICK); // IRQ enable
	SysTick_IntEnable();	// interrupt enable
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
