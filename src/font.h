/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _FONT_H
#define _FONT_H

#include "gfx.h"

//Font types
typedef enum
{
	Font_Bold,
	Font_Arial,
} Font;

typedef enum
{
	FontAlign_Left,
	FontAlign_Center,
	FontAlign_Right,
} FontAlign;

typedef struct FontData
{
	//Font functions and data
	s32 (*get_width)(struct FontData *this, const char *text);
	void (*draw_col)(struct FontData *this, const char *text, s32 x, s32 y, FontAlign align, u8 r, u8 g, u8 b);
	void (*draw)(struct FontData *this, const char *text, s32 x, s32 y, FontAlign align);
	
	Gfx_Tex tex;
} FontData;

//Font functions
void FontData_Load(FontData *this, Font font);

#endif
