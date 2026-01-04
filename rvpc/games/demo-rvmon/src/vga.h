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

#ifndef __VGA_H
#define __VGA_H

#define VGA_NUM_COLS WIDTH
#define VGA_NUM_ROWS HEIGHT
#define VGA_CURSOR

u8 vga_is_frame_end();

void vga_cls();
void vga_clear_rect(u8 r1, u8 c1, u8 r2, u8 c2);

#ifdef VGA_CURSOR
void vga_cursor_hide();
void vga_cursor_set(u8 row, u8 col);
u8 vga_cursor_row();
u8 vga_cursor_col();
#endif

char vga_char_at(u8 row, u8 col);
void vga_write_at(u8 row, u8 col, char ch);
void vga_print_at(u8 row, u8 col, const char* text);

void vga_printf_at(u8 row, u8 col, const char* format, ...) __attribute__((__format__ (__printf__, 3, 4)));;


/*********************************************************************
 * @fn    vga_scroll_up
 * @brief Scroll one row up
 */
#define vga_scroll_up() PrintScroll()

/*********************************************************************
 * @fn    vga_char_at
 * @brief Returns char displayed at position (row, col)
 * @param row - position row
 * @param col - position col
 */
#define vga_char_at(row, col) FrameBuf[row * col]

/*********************************************************************
 * @fn    vga_write_at
 * @brief Write char at position (row, col)
 * @param row - position row
 * @param col - position col
 * @param ch  - char to diaplay
 */
#define vga_write_at(row, col, ch) PrintCharAt(ch, col, row, COL_WHITE)

/*********************************************************************
 * @fn    vga_print_at
 * @brief Print text at position (row, col)
 * @param row  position row
 * @param col  position col
 * @param text pointer to null terminated text to diaplay
 */
#define vga_print_at(row, col, text) PrintTextAt(text, col, row, COL_WHITE)

#endif // __VGA_H