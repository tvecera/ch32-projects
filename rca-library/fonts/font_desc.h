// ============================================================================
// Font Descriptor for Text Mode Video Output
// ============================================================================
//
// Defines font structure for text video modes (VMODE 6, 7, 8).
// Supports both standard ASCII-mapped fonts and custom character sets.
//
// Mapping strategy:
// - Standard fonts: quick math (ch - ascii_offset) & mask
// - Custom fonts: 128-byte lookup table in Flash
// - Mapping is done in print.c, NOT in IRQ handler
// - FrameBuf always contains font indices (0-63 or 0-127)
// - IRQ handler uses direct index without mapping
//
// ============================================================================
#ifndef _FONT_DESC_H
#define _FONT_DESC_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Font Descriptor Structure
// ============================================================================

typedef struct {
    const unsigned char* data; // Font bitmap data pointer (in Flash)
    const unsigned char* ascii_map; // 128B ASCII->index lookup table, or NULL for standard
    unsigned char char_count; // Number of characters: 64 or 128
    unsigned char space_idx; // Precomputed index for space character (for fast screen clear)
} font_desc_t;

// ============================================================================
// Standard ASCII Mapping Table for 64-character fonts (in Flash)
// ============================================================================
//
// Maps ASCII 0-127 to font indices 0-63:
// - ASCII 0-31 (control chars) -> 0 (space)
// - ASCII 32-95 (printable) -> 0-63 (direct mapping)
// - ASCII 96-122 (lowercase) -> 33-58 (map to uppercase A-Z)
// - ASCII 123-127 -> 0 (space)
//
static const unsigned char ascii_map_standard64[128] = {
    // 0x00-0x0F: control characters -> space (index 0)
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // 0x10-0x1F: control characters -> space (index 0)
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // 0x20-0x2F: ' ' to '/' -> index 0-15
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    // 0x30-0x3F: '0' to '?' -> index 16-31
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    // 0x40-0x4F: '@' to 'O' -> index 32-47
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    // 0x50-0x5F: 'P' to '_' -> index 48-63
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    // 0x60-0x6F: '`' and 'a'-'o' -> backtick=32(@), lowercase=uppercase
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    // 0x70-0x7F: 'p'-'z' and special -> lowercase=uppercase, rest=space
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 0, 0, 0, 0, 0,
};

// ============================================================================
// ZX-81 ASCII Mapping Table for ZX-80/ZX-81 64-character fonts
// ============================================================================
//
// ZX-81 character set layout:
//   0:      space
//   1-10:   graphics blocks (inverse patterns)
//   11:     "
//   12:     £ (pound)
//   13:     $
//   14:     :
//   15:     ?
//   16-17:  ( )
//   18-19:  > <
//   20-21:  = +
//   22-24:  - * /
//   25-27:  ; , .
//   28-37:  0-9
//   38-63:  A-Z
//
static const unsigned char ascii_map_zx81[128] = {
    // 0x00-0x0F: control characters -> space (index 0)
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // 0x10-0x1F: control characters -> space (index 0)
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // 0x20-0x2F: ' ' !"#$%&'()*+,-./
    0, 0, 11, 0, 13, 0, 0, 0, 16, 17, 23, 21, 26, 22, 27, 24,
    // 0x30-0x3F: 0-9 :;<=>?
    28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 14, 25, 19, 20, 18, 15,
    // 0x40-0x4F: @ A-O
    0, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
    // 0x50-0x5F: P-Z [\]^_
    53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 0, 0, 0, 0,
    // 0x60-0x6F: ` a-o -> uppercase
    0, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
    // 0x70-0x7F: p-z {|}~ DEL -> uppercase / space
    53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 0, 0, 0, 0,
};

// NULL
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif // _FONT_DESC_H
