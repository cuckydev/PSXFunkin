#include "speaker.h"

#include "../io.h"
#include "../stage.h"

//Speaker functions
void Speaker_Init(Speaker *this)
{
	//Initialize speaker state
	this->bump = 0;
	
	//Load speaker graphics
	Gfx_LoadTex(&this->tex, IO_Read("\\GF\\SPEAKER.TIM;1"), GFX_LOADTEX_FREE);
}

void Speaker_Bump(Speaker *this)
{
	//Set bump
	this->bump = 8;
}

void Speaker_Tick(Speaker *this, fixed_t x, fixed_t y)
{
	//Get frame to use according to bump
	u8 frame;
	if (this->bump != 0)
		frame = (this->bump-- + 3) >> 2;
	else
		frame = 0;
	
	//Draw speakers
	static const struct SpeakerPiece
	{
		u8 rect[4];
		u8 ox, oy;
	} speaker_draw[3][2] = {
		{ //bump 0
			{{ 96,  88, 160, 88},   0,  0},
			{{  0, 176,  16, 56}, 160, 32},
		},
		{ //bump 1
			{{176,   0,  80, 88},   0,  0},
			{{  0,  88,  96, 88},  80,  0},
		},
		{ //bump 2
			{{  0,   0,  88, 88},   0,  0},
			{{ 88,   0,  88, 88},  88,  0},
		}
	};
	
	const struct SpeakerPiece *piece = speaker_draw[frame];
	for (int i = 0; i < 2; i++, piece++)
	{
		//Draw piece
		RECT piece_src = {piece->rect[0], piece->rect[1], piece->rect[2], piece->rect[3]};
		if ((piece_src.x + piece_src.w) >= 0x100)
			piece_src.w = 0xFF - piece_src.x;
		RECT_FIXED piece_dst = {
			x - FIXED_DEC(88,1) + ((fixed_t)piece->ox << FIXED_SHIFT) - stage.camera.x,
			y + ((fixed_t)piece->oy << FIXED_SHIFT) - stage.camera.y,
			(fixed_t)piece->rect[2] << FIXED_SHIFT,
			(fixed_t)piece->rect[3] << FIXED_SHIFT,
		};
		
		Stage_DrawTex(&this->tex, &piece_src, &piece_dst, stage.camera.bzoom);
	}
}
