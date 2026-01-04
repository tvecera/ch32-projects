/**
 * Ported from Olimex Retro-Computers project to CH32LibSDK (Tomas Vecera 2025)
 *
 * This file is part of the RVPC project
 * https://github.com/OLIMEX/RVPC
 * https://www.olimex.com/Products/Retro-Computers/RVPC/open-source-hardware
 *
 * Copyright (c) 2024 Olimex Ltd
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
#include "font.c"

extern void initialize_application();
extern void run_keyboard_state_machine();
extern void run_app_state_machine();

int main(void) {
#if USE_RCA
    SetFont(&TextModeFontRvpcTetris);
#else
    SetFont(TextModeFontRvpcTetris);
#endif
    initialize_application();
    while (1) {
        run_keyboard_state_machine();
        if (vga_is_frame_end()) {
            run_app_state_machine();
        }
    }
}


