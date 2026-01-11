/**
 * ============================================================================
 * Main Include File
 * ============================================================================
 *
 * Aggregates all project headers. Include this file from source files
 * instead of individual headers.
 *
 * Handles framework detection (CH32LibSDK vs ch32fun) and includes
 * appropriate headers for each.
 *
 * ============================================================================
 */
#ifndef CH32FUN
#include INCLUDES_H     // all includes
#include "rca_config.h"
#else
extern const __attribute__((aligned(4))) unsigned char FontBold8x8[2048];
extern const __attribute__((aligned(4))) unsigned char FontThin8x8[2048];
#include "ch32fun.h"
#include "src/ch32libsdk.h"
#include "src/timer.h"
#endif

#include "src/rca_video.h"

#if VMODE > 1
#include "fonts/font_desc.h"

#if USE_RCA_SPI
#define TextModeFontThin8x8 TextModeFontThin8x8Spi
#define TextModeFontBold8x8 TextModeFontBold8x8Spi
#define TextModeFontThin8x8_64 TextModeFontThin8x8Spi64
#define TextModeFontBold8x8_64 TextModeFontBold8x8Spi64
#define TextModeFontRvpc TextModeFontRvpcSpi
#define TextModeFontZx TextModeFontZxSpi
#define TextModeFont80 TextModeFont80Spi
#define TextModeFont81 TextModeFont81Spi
#else
#define TextModeFontThin8x8 TextModeFontThin8x8BitBang
#define TextModeFontBold8x8 TextModeFontBold8x8BitBang
#define TextModeFontThin8x8_64 TextModeFontThin8x8BitBang64
#define TextModeFontBold8x8_64 TextModeFontBold8x8BitBang64
#define TextModeFontRvpc TextModeFontRvpcBitBang
#define TextModeFontZx TextModeFontZxBitBang
#define TextModeFont80 TextModeFont80BitBang
#define TextModeFont81 TextModeFont81BitBang
#endif

extern const font_desc_t TextModeFontThin8x8;
extern const font_desc_t TextModeFontBold8x8;
extern const font_desc_t TextModeFontThin8x8_64;
extern const font_desc_t TextModeFontBold8x8_64;
extern const font_desc_t TextModeFontRvpc;
extern const font_desc_t TextModeFontZx;
extern const font_desc_t TextModeFont80;
extern const font_desc_t TextModeFont81;

#endif
