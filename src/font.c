/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "font.h"

#include "timer.h"

#include <string.h>

//Font_Bold
s32 Font_Bold_GetWidth(struct FontData *this, const char *text)
{
	(void)this;
	return strlen(text) * 13;
}

void Font_Bold_Draw(struct FontData *this, const char *text, s32 x, s32 y, FontAlign align)
{
	//Offset position based off alignment
	switch (align)
	{
		case FontAlign_Left:
			break;
		case FontAlign_Center:
			x -= Font_Bold_GetWidth(this, text) >> 1;
			break;
		case FontAlign_Right:
			x -= Font_Bold_GetWidth(this, text);
			break;
	}
	
	//Get animation offsets
	u32 v0 = 0;
	u8 v1 = (animf_count >> 1) & 1;
	
	//Draw string character by character
	u8 c;
	while ((c = *text++) != '\0')
	{
		//Draw character
		if ((c -= 'A') <= 'z' - 'A') //Lower-case will show inverted colours
		{
			RECT src = {((c & 0x7) << 5) + ((((v0 >> (c & 0x1F)) & 1) ^ v1) << 4), (c & ~0x7) << 1, 16, 16};
			Gfx_BlitTex(&this->tex, &src, x, y);
			v0 ^= 1 << (c & 0x1F);
		}
		x += 13;
	}
}

//Font functions
void FontData_Load(FontData *this, Font font)
{
	//Load the given font
	switch (font)
	{
		case Font_Bold:
			//Load texture and set functions
			Gfx_LoadTex(&this->tex, IO_Read("\\FONT\\BOLDFONT.TIM;1"), GFX_LOADTEX_FREE);
			this->get_width = Font_Bold_GetWidth;
			this->draw = Font_Bold_Draw;
			break;
	}
}
