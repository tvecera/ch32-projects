
// ****************************************************************************
//
//                        RVPC - VGA display driver
//
// ****************************************************************************
// Using system clock 50MHz with HSI internal oscillator or external crystal 25MHz.
#if USE_DISP		// 1=use display support

#if !USE_RCA

#ifndef _RVPC_VGA_H
#define _RVPC_VGA_H

#ifdef __cplusplus
extern "C" {
#endif

// Videomodes

#ifdef CH32V003
STATIC_ASSERT(VMODE == 0 || VMODE == 8, "CH32V003 supports VMODE 0 or 8.");
#endif

#if USE_RCA
STATIC_ASSERT(VMODE == 0 || VMODE == 1 || VMODE == 8, "RCA supports VMODE 0, 1 or 8.");
#endif

// Videomode 0: graphics mode 128x64 pixels, required memory 1024 B
#if VMODE == 0
#define WIDTH		128		// width in pixels
#define HEIGHT		64		// height in graphics lines (must be even number - due render "next scanline")
#define WIDTHBYTE	(WIDTH/8)	// width in bytes (= 16)
#define FRAMESIZE	(WIDTHBYTE*HEIGHT) // size of frame buffer in bytes (= 16*64 = 1024 bytes)
#define TEXTWIDTH	(WIDTH/8)	// text width in characters (= 16)
#define TEXTHEIGHT	(HEIGHT/8)	// text height in rows (= 8; 1 character = 8x8 pixels)

// Videomode 1: graphics mode 160x120 pixels, required memory 2400 B
#elif VMODE == 1
#define WIDTH		160		// width in pixels
#define HEIGHT		120		// height in graphics lines (must be even number - due render "next scanline")
#define WIDTHBYTE	(WIDTH/8)	// width in bytes (= 20)
#define FRAMESIZE	(WIDTHBYTE*HEIGHT) // size of frame buffer in bytes (= 20*120 = 2400 bytes)
#define TEXTWIDTH	(WIDTH/8)	// text width in characters (= 20)
#define TEXTHEIGHT	(HEIGHT/8)	// text height in rows (= 15; 1 character = 8x8 pixels)

// Videomode 8:text mode 23x18 characters of 8x8 pixels with custom fonts of 64/128 characters
// required memory 414 bytes, font 8x8 pixels 4096/8192 bytes in Flash
#elif VMODE == 8
#define WIDTH		23		// width in characters
#define HEIGHT		18		// height in text rows
#define WIDTHBYTE	WIDTH		// width in bytes (= 23)
#define FRAMESIZE	(WIDTHBYTE*HEIGHT) // size of frame buffer in bytes (= 23*18 = 414 bytes)
#define TEXTWIDTH	WIDTH		// text width in characters (= 23)
#define TEXTHEIGHT	HEIGHT		// text height in rows (= 18; 1 character = 8x8 pixels)

#endif // VMODE=...

extern u8 FrameBuf[FRAMESIZE];		// display graphics buffer
extern volatile u32 DispLine;		// current display line
extern volatile u32 DispFrame;		// current frame
extern volatile u8* FrameBufAddr;	// current pointer to graphics buffer
extern volatile u32 DispTimTest;	// test - get TIM-CNT value at start of image

// check VSYNC
#if VMODE == 0
INLINE Bool DispIsVSync() { return DispLine >= 256; }
#elif VMODE == 1 && USE_RCA == 1
INLINE Bool DispIsVSync() { return DispLine >= 312; }
#elif VMODE == 8
INLINE Bool DispIsVSync() { return DispLine >= 576; }
#else
INLINE Bool DispIsVSync() { return DispLine >= 480; }
#endif
// wait for VSync scanline
void WaitVSync();

// Initialize videomode
void DispInit();

// Terminate videomode
void DispTerm();

#ifdef __cplusplus
}
#endif

#endif // _RVPC_VGA_H

#endif // USE_DISP

#endif // USE_RCA
