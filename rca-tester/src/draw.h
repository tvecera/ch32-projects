/**
 * ============================================================================
 * Graphics Drawing Library for Monochrome Framebuffer
 * ============================================================================
 *
 * Provides 2D graphics primitives for 1-bit framebuffer rendering.
 * Used in graphics video modes (VMODE 0, 1).
 *
 * Based on drawing functions from PicoLibSDK / CH32LibSDK
 * Original: https://github.com/Panda381/PicoLibSDK
 *           https://github.com/Panda381/CH32LibSDK
 * Author:   Miroslav Nemecek
 *
 * Features:
 * - Pixel operations (set, clear, with/without bounds check)
 * - Text rendering (requires FONT definition)
 *
 * ============================================================================
 */

#ifndef _DRAW_H
#define _DRAW_H

#ifdef __cplusplus
extern "C" {

#endif

#if VMODE == 0 || VMODE == 1

// ============================================================================
// Color Constants
// ============================================================================

#define COL_BLACK   0
#define COL_WHITE   1

// ============================================================================
// Framebuffer Functions
// ============================================================================

/**
 * @brief Clear framebuffer (set all pixels to black)
 */
void DrawClear();

// ============================================================================
// Pixel Functions
// ============================================================================

/**
 * @brief Set or clear pixel without bounds checking
 * @param x X coordinate (0..WIDTH-1)
 * @param y Y coordinate (0..HEIGHT-1)
 * @param col Color (COL_BLACK=0 or COL_WHITE=1)
 * @warning No bounds checking - caller must ensure valid coordinates
 */
void DrawPointFast(int x, int y, u8 col);

/**
 * @brief Clear pixel without bounds checking
 * @param x X coordinate (0..WIDTH-1)
 * @param y Y coordinate (0..HEIGHT-1)
 * @warning No bounds checking - caller must ensure valid coordinates
 */
void DrawPointClrFast(int x, int y);

/**
 * @brief Set or clear pixel with bounds checking
 * @param x X coordinate (can be out of bounds - will be ignored)
 * @param y Y coordinate (can be out of bounds - will be ignored)
 * @param col Color (COL_BLACK=0 or COL_WHITE=1)
 */
void DrawPoint(int x, int y, u8 col);

/**
 * @brief Clear pixel with bounds checking
 * @param x X coordinate (can be out of bounds - will be ignored)
 * @param y Y coordinate (can be out of bounds - will be ignored)
 */
void DrawPointClr(int x, int y);

// ============================================================================
// Text Functions (requires FONT definition)
// ============================================================================

#ifdef FONT

/** @brief Pointer to current font data */
extern const u8* volatile DrawFont;

/** @brief Invert text rendering (0=normal, non-zero=inverted) */
extern u8 PrintInv;

/**
 * @brief Draw single character at specified position
 * @param ch Character to draw (ASCII)
 * @param x X coordinate in pixels
 * @param y Y coordinate in pixels
 * @param col Text color (COL_WHITE=1 for visible text)
 */
void DrawChar(char ch, int x, int y, u8 col);

/**
 * @brief Draw null-terminated string at specified position
 * @param text String to draw
 * @param x Starting X coordinate in pixels
 * @param y Y coordinate in pixels
 * @param col Text color (COL_WHITE=1 for visible text)
 */
void DrawText(const char* text, int x, int y, u8 col);

#endif // FONT

#endif // VMODE == 0 || VMODE == 1

#ifdef __cplusplus
}
#endif

#endif
