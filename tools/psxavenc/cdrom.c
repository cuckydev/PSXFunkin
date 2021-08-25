/*
psxavenc: MDEC video + SPU/XA-ADPCM audio encoder frontend

Copyright (c) 2019, 2020 Adrian "asie" Siekierka
Copyright (c) 2019 Ben "GreaseMonkey" Russell

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

This tool has been modified so that libpsxav isn't an external library
*/

#include "common.h"

void init_sector_buffer_video(uint8_t *buffer, settings_t *settings) {
	memset(buffer,0,2352);
	memset(buffer+0x001,0xFF,10);

	buffer[0x00F] = 0x02;
	buffer[0x010] = settings->file_number;
	buffer[0x011] = settings->channel_number & 0x1F;
	buffer[0x012] = 0x08 | 0x40;
	buffer[0x013] = 0x00;
	memcpy(buffer + 0x014, buffer + 0x010, 4);
}

void calculate_edc_data(uint8_t *buffer)
{
	uint32_t edc = 0;
	for (int i = 0x010; i < 0x818; i++) {
		edc ^= 0xFF&(uint32_t)buffer[i];
		for (int ibit = 0; ibit < 8; ibit++) {
			edc = (edc>>1)^(0xD8018001*(edc&0x1));
		}
	}
	buffer[0x818] = (uint8_t)(edc);
	buffer[0x819] = (uint8_t)(edc >> 8);
	buffer[0x81A] = (uint8_t)(edc >> 16);
	buffer[0x81B] = (uint8_t)(edc >> 24);

	// TODO: ECC
}
