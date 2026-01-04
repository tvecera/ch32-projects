/**
 * Ported from Olimex Retro-Computers project to CH32LibSDK (Tomas Vecera 2025)
 *
 * This file is part of the RVPC project 
 * https://github.com/OLIMEX/RVPC
 * https://www.olimex.com/Products/Retro-Computers/RVPC/open-source-hardware
 * 
 * Copyright (c) 2024 Olimex Ltd
 * Copyright (c) 2024 Curtis Whitley
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../include.h"

static u32 last_frame = 0;
static uint8_t s_row = 0xFF;
static uint8_t s_col = 0xFF;
static uint16_t s_line = 0xFFFF;

/*********************************************************************
 * @fn    vga_is_frame_end
 * @brief Check if VGA/RCA frame is ended
 */
inline uint8_t vga_is_frame_end() {
    u32 current_frame = DispFrame;
    if (current_frame != last_frame) {
        last_frame = current_frame;
        return 1;
    }
    return 0;
}

/*********************************************************************
 * @fn    vga_cls
 * @brief Clear VGA screen
 */
void vga_cls() {
    memset(FrameBuf, ' ', FRAMESIZE);
    PrintPos = 0;
    PrintRow = 0;
    PrintCol = COL_WHITE;
    PrintInv = 0;
}

/*********************************************************************
 * @fn    vga_clear_rect
 * @brief Clear VGA rectangle
 * @param r1 - from row
 * @param c1 - from col
 * @param r2 - to row
 * @param c2 - to col
 */
void vga_clear_rect(const uint8_t r1, const uint8_t c1, const uint8_t r2, const uint8_t c2) {
    if (r1 > r2 || c1 > c2) {
        return;
    }
    for (uint8_t r = r1; r <= r2; r++) {
        memset(&FrameBuf[r * WIDTH], 0x20, (c2 - c1 + 1));
    }
}

/*********************************************************************
 * @fn    vga_cursor_hide
 * @brief Hide the blinking cursor
 */
void vga_cursor_hide() {
    s_row = 0xFF;
    s_col = 0xFF;
    s_line = 0xFFFF;
}

/*********************************************************************
 * @fn    vga_cursor_set
 * @brief Show the blinking cursor at positon (row, col)
 * @param row - position row
 * @param col - position col
 */
void vga_cursor_set(const uint8_t row, const uint8_t col) {
    if (row < VGA_NUM_ROWS && col < VGA_NUM_COLS) {
        s_row = row;
        s_col = col;
        s_line = (row << 3) + 7;
    } else {
        vga_cursor_hide();
    }
}

/*********************************************************************
 * @fn    vga_printf_at
 * @brief Print formatted text at position (row, col)
 * @param row  position row
 * @param col  position col
 * @param format pointer to null terminated formatted text to diaplay
 */
void vga_printf_at(const uint8_t row, const uint8_t col, const char* format, ...) {
    va_list argptr;
    char tmp[VGA_NUM_COLS + 2];

    va_start(argptr, format);
    vsnprintf(tmp, sizeof(tmp), format, argptr);
    va_end(argptr);

    vga_print_at(row, col, tmp);
}

/*********************************************************************
 * @fn    vga_cursor_row
 * @brief Get cursor position row
 */
uint8_t vga_cursor_row() {
    return s_row;
}

/*********************************************************************
 * @fn    vga_cursor_col
 * @brief Get cursor position col
 */
uint8_t vga_cursor_col() {
    return s_col;
}
