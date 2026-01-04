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

const sMelodyNote OkSound[] = {
    {NOTE_LEN32DOT, NOTE_F5},
    {NOTE_LEN32DOT, NOTE_A5},
    {NOTE_LEN32DOT, NOTE_C6},
    {NOTE_LEN32DOT, NOTE_F6},
    {0, 0},
};

void buzz(uint32_t hz, uint32_t timeMS) {
	#ifdef KBD_USE_BUZZ
	PlayTone(SND_GET_DIV(hz * 100));
	WaitMs(timeMS);
	StopSound();
	#endif
}

void buzz_ok() {
	#ifdef KBD_USE_BUZZ
    PlayMelody(OkSound);
	#endif
}

uint32_t kbd_wait() {
	uint32_t code = 0;
	do {
		code = kbd_read();
	} while (code == 0x0000);
	return code;
}

uint32_t kbd_wait_press() {
	uint32_t code = 0;
	do {
		code = kbd_read();
	} while (code == 0 || (code & 0xFF00) == 0xF000);
	return code;
}

uint32_t kbd_wait_release() {
	uint32_t code = 0;
	do {
		code = kbd_read();
	} while ((code & 0xFF00) != 0xF000);
	return code;
}
