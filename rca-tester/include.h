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
#else
#include "config.h"
#include "ch32fun.h"
#endif

#include "../rca-library/include.h"
#include "src/main.h"

#if USE_CH32BASE || CH32FUN

#if USE_KEYS
#include "src/key.h"
#endif

#if USE_SOUND
#include "src/snd.h"
#endif

#if VMODE == 0 || VMODE == 1
#include "src/draw.h"
#endif

#if VMODE > 1
#include "src/print.h"
#endif

#endif
