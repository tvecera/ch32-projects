
// ****************************************************************************
//
//                        RVPC - Device init
//
// ****************************************************************************
#include INCLUDES_H     // all includes
#include "_include.h"

#if USE_RCA
INLINE void DispInit() { rca_init(); }

INLINE void DispTerm() { rca_stop(); }

void frame_callback() {
	SoundScan();
}
#endif

// Initialize device
void DevInit()
{
	// It's necessary to set 0 wait states for Flash memory when using VGA output. Without this, the display will
	// be unstable.
 #if CH32V003
 	Flash_Latency(0);
 #endif

#if USE_KEYS		// 1=use keyboard support
	// Initialize keyboard service (should be called before display init)
	KeyInit();
#endif

#if USE_SOUND		// 1=use sound support
	// Sound initialize
	SoundInit();
#if USE_RCA
	rca_set_frame_callback(frame_callback);
#endif
#endif

#if USE_DISP		// 1=use display support
	// Initialize videomode
	DispInit();
#endif

#if USE_DRAW || USE_PRINT
	// clear screen
	DrawClear();
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
	// Terminate videomode
	DispTerm();
#endif

#if USE_SOUND		// 1=use sound support
	// Sound terminate
	SoundTerm();
#endif

#if USE_KEYS		// 1=use keyboard support
	// terminate keyboard
	KeyTerm();
#endif
}
