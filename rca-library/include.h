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

#if VMODE > 5
#include "fonts/font_desc.h"

// 128-char fonts
extern const font_desc_t TextModeFontThin8x8;
extern const font_desc_t TextModeFontBold8x8;
extern const font_desc_t TextModeFontRvpc;
extern const font_desc_t TextModeFontZx;

// 64-char fonts
extern const font_desc_t TextModeFontThin8x8_64;
extern const font_desc_t TextModeFontBold8x8_64;
extern const font_desc_t TextModeFont80;
extern const font_desc_t TextModeFont81;
#endif
