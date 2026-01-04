
// ****************************************************************************
//                                 
//                        Project library configuration
//
// ****************************************************************************

#ifndef _CONFIG_H
#define _CONFIG_H

// default font
#define FONT		FontCond6x6 //FontBold8x8	// default system font
#define FONTCOND	FontCond6x6	// default condensed font

// ============================================================================
// Video Mode Selection
// ============================================================================
// Available video modes (B&W monochrome):
//
// Graphics modes (FrameBuf = pixels):
//   VMODE 0: 128x64 px,  RAM: 1024B buf + 306B handler = 1330B total
//   VMODE 1: 160x120 px, RAM: 2400B buf + 330B handler = 2730B total
//
// Text modes (FrameBuf = ASCII characters, requires font in Flash):
//   VMODE 6: 40x30 chars (8x8 font),  RAM: 1200B buf + 20B state + 376B handler = 1596B
//   VMODE 7: 32x24 chars (8x8 font),  RAM: 768B buf + 20B state + 430B handler = 1218B
//   VMODE 8: 23x18 chars (8x14 font), RAM: 414B buf + 64B state + 418B handler = 896B
//
// Font Flash usage: 64 chars = 2048B, 128 chars = 4096B
#define VMODE           1

// Video output method:
//   USE_RCA_SPI = 0: Bit-banging on PD6 (default, works on all pins)
//   USE_RCA_SPI = 1: SPI1 MOSI on PC6 (hardware shift, frees CPU during pixel output)
//
// SPI mode requires PC6 for video output (SPI1_MOSI default pin)
// Bit-bang mode uses PD6 (can be changed in rca.c)
#define USE_RCA_SPI     0

#define SOUND_MUTE  0

// ----------------------------------------------------------------------------
//                            Library modules
// ----------------------------------------------------------------------------

#define USE_CRC		0	// 1=use CRC library
#define USE_DECNUM	1	// 1=use decode number
#define USE_FAT		0	// 1=use FAT filesystem
#define USE_RAND	1	// 1=use random number generator
#define USE_SD		0	// 1=use SD card driver

// ----------------------------------------------------------------------------
//                             Device setup
// ----------------------------------------------------------------------------

#define USE_DRAW	1	// 1=use graphics drawing functions
#define USE_PRINT	0	// 1=use text printing functions
#define USE_KEYS	1	// 1=use keyboard support
#define USE_SOUND	1	// 1=use sound support
#define USE_DISP	1	// 1=use display support

#include "../../src/_config.h"

#if USE_RCA
#include "../../../rca-library/rca_config.h"
#endif

#endif // _CONFIG_H
