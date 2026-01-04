#if USE_RCA
#include "../../../rca-library/include.h"
#else
#include "rvpc_vga.h"	// VGA display driver

#if FONT_CHAR_COUNT == 64
extern const ALIGNED u8 TextModeFont80[2048];
extern const ALIGNED u8 TextModeFont81[2048];
extern const ALIGNED u8 TextModeFontThin8x8[2048];
extern const ALIGNED u8 TextModeFontBold8x8[2048];
#elif FONT_CHAR_COUNT == 128
extern const ALIGNED u8 TextModeFontZx[4096];
extern const ALIGNED u8 TextModeFontRvpc[4096];
extern const ALIGNED u8 TextModeFontThin8x8[4096];
extern const ALIGNED u8 TextModeFontBold8x8[4096];
#endif

#endif

#include "rvpc_draw.h"	// drawing
#include "rvpc_key.h"   // PS/2 keyboard
#include "rvpc_snd.h"	// sound
#include "rvpc_init.h"	// device init
