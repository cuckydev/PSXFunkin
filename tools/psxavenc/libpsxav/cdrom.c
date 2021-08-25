/*
libpsxav: MDEC video + SPU/XA-ADPCM audio library

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
*/

#include <string.h>
#include "libpsxav.h"

static uint32_t psx_cdrom_calculate_edc(uint8_t *sector, uint32_t offset, uint32_t size)
{
	uint32_t edc = 0;
	for (int i = offset; i < offset+size; i++) {
		edc ^= 0xFF&(uint32_t)sector[i];
		for (int ibit = 0; ibit < 8; ibit++) {
			edc = (edc>>1)^(0xD8018001*(edc&0x1));
		}
	}
	return edc;
}

void psx_cdrom_calculate_checksums(uint8_t *sector, psx_cdrom_sector_type_t type)
{
	switch (type) {
		case PSX_CDROM_SECTOR_TYPE_MODE1: {
			uint32_t edc = psx_cdrom_calculate_edc(sector, 0x0, 0x810);
			sector[0x810] = (uint8_t)(edc);
			sector[0x811] = (uint8_t)(edc >> 8);
			sector[0x812] = (uint8_t)(edc >> 16);
			sector[0x813] = (uint8_t)(edc >> 24);

			memset(sector + 0x814, 0, 8);
			// TODO: ECC
		} break;
		case PSX_CDROM_SECTOR_TYPE_MODE2_FORM1: {
			uint32_t edc = psx_cdrom_calculate_edc(sector, 0x10, 0x808);
			sector[0x818] = (uint8_t)(edc);
			sector[0x819] = (uint8_t)(edc >> 8);
			sector[0x81A] = (uint8_t)(edc >> 16);
			sector[0x81B] = (uint8_t)(edc >> 24);

			// TODO: ECC
		} break;
		case PSX_CDROM_SECTOR_TYPE_MODE2_FORM2: {
			uint32_t edc = psx_cdrom_calculate_edc(sector, 0x10, 0x91C);
			sector[0x92C] = (uint8_t)(edc);
			sector[0x92D] = (uint8_t)(edc >> 8);
			sector[0x92E] = (uint8_t)(edc >> 16);
			sector[0x92F] = (uint8_t)(edc >> 24);
		} break;
	}
}