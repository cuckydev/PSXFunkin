/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week3.h"

#include "../mem.h"
#include "../archive.h"
#include "../random.h"
#include "../timer.h"

//Week 3 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //Buildings
	Gfx_Tex tex_back1; //Lights
	Gfx_Tex tex_back2; //Rooftop
	Gfx_Tex tex_back3; //Background train arc
	Gfx_Tex tex_back4; //Train
	Gfx_Tex tex_back5; //Sky
	
	//Window state
	u8 win_r, win_g, win_b;
	fixed_t win_time;
	
	//Train state
	fixed_t train_x;
	u8 train_timer;
} Back_Week3;

//Week 3 background functions
static const u8 win_cols[][3] = {
	{ 49, 162, 253},
	{ 49, 253, 140},
	{251,  51, 245},
	{253,  69,  49},
	{251, 166,  51},
};

#define TRAIN_START_X FIXED_DEC(500,1)
#define TRAIN_END_X    FIXED_DEC(-8000,1)
#define TRAIN_TIME_A 6
#define TRAIN_TIME_B 14

void Back_Week3_Window(Back_Week3 *this)
{
	const u8 *win_col = win_cols[RandomRange(0, COUNT_OF(win_cols) - 1)];
	this->win_r = win_col[0];
	this->win_g = win_col[1];
	this->win_b = win_col[2];
	this->win_time = FIXED_DEC(3,1);
}

void Back_Week3_DrawBG(StageBack *back)
{
	Back_Week3 *this = (Back_Week3*)back;
	
	fixed_t fx, fy;
	
	//Update window
	if (this->win_time > 0)
	{
		this->win_time -= timer_dt;
		if (this->win_time < 0)
			this->win_time = 0;
	}
	if (stage.note_scroll >= 0 && (stage.flag & STAGE_FLAG_JUST_STEP) && (stage.song_step & 0xF) == 0)
		Back_Week3_Window(this);
	
	//Draw rooftop
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	static const struct Back_Week3_RoofPiece
	{
		RECT src;
		fixed_t scale;
	} roof_piece[] = {
		{{  0, 0,  16, 256}, FIXED_MUL(FIXED_DEC(3,1) * 7 / 10, FIXED_UNIT + FIXED_DEC(SCREEN_WIDEOADD,2) * 10 / 336)},
		{{ 16, 0,  55, 256}, FIXED_DEC(1,1) * 9 / 10},
		{{ 71, 0, 128, 256}, FIXED_DEC(265,100) * 7 / 10},
		{{199, 0,  55, 256}, FIXED_DEC(1,1) * 9 / 10},
		{{255, 0,   0, 256}, FIXED_DEC(16,1) + FIXED_DEC(SCREEN_WIDEOADD2,1)}
	};
	
	RECT_FIXED roof_dst = {
		FIXED_DEC(-210,1) - FIXED_DEC(SCREEN_WIDEOADD,2) - fx,
		FIXED_DEC(-106,1) - fy,
		0,
		FIXED_DEC(220,1)
	};
	
	const struct Back_Week3_RoofPiece *roof_p = roof_piece;
	for (size_t i = 0; i < COUNT_OF(roof_piece); i++, roof_p++)
	{
		roof_dst.w = roof_p->src.w ? (roof_p->src.w * roof_p->scale) : roof_p->scale;
		Stage_DrawTex(&this->tex_back2, &roof_p->src, &roof_dst, stage.camera.bzoom);
		roof_dst.x += roof_dst.w;
	}
	
	RECT roof_fillsrc = {0, 254, 1, 0};
	RECT roof_fill = {0, SCREEN_HEIGHT * 2 / 3, SCREEN_WIDTH, SCREEN_HEIGHT * 1 / 3};
	Gfx_DrawTex(&this->tex_back2, &roof_fillsrc, &roof_fill);
	
	//Move train
	if (this->train_x <= TRAIN_END_X)
	{
		//Reset train
		if ((stage.flag & STAGE_FLAG_JUST_STEP) && (stage.song_step & 0xF) == 0)
		{
			if (--this->train_timer == 0)
			{
				this->train_x = TRAIN_START_X;
				this->train_timer = RandomRange(TRAIN_TIME_A, TRAIN_TIME_B);
			}
		}
	}
	else
	{
		//Move train to end position
		this->train_x  -= timer_dt * 2000;
		
		//Draw train
		RECT train_src = {0, 0, 256, 256};
		RECT_FIXED train_dst = {
			this->train_x - fx,
			FIXED_DEC(-65,1) - fy,
			FIXED_DEC(285,1),
			FIXED_DEC(120,1)
		};
		
		for (int i = 0; i < 24; i++, train_dst.x += train_dst.w)
		{
			if (train_dst.x >= (SCREEN_WIDTH2 << FIXED_SHIFT) || train_dst.x <= -(train_dst.w + (SCREEN_WIDTH2 << FIXED_SHIFT)))
				continue;
			Stage_DrawTex(&this->tex_back4, &train_src, &train_dst, stage.camera.bzoom);
		}
	}
	
	//Draw arc
	RECT arcl_src = {0, 0, 38, 121};
	RECT_FIXED arcl_dst = {
		FIXED_DEC(-131,1) - fx,
		FIXED_DEC(-86,1) - fy,
		FIXED_DEC(38,1),
		FIXED_DEC(121,1)
	};
	
	RECT arcr_src = {39, 0, 39, 121};
	RECT_FIXED arcr_dst = {
		FIXED_DEC(74,1) - fx,
		FIXED_DEC(-85,1) - fy,
		FIXED_DEC(39,1),
		FIXED_DEC(121,1)
	};
	
	Stage_DrawTex(&this->tex_back3, &arcl_src, &arcl_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back3, &arcr_src, &arcr_dst, stage.camera.bzoom);
	
	//Draw lights
	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;
	
	if (this->win_time >= 0)
	{
		RECT lightl_src = {0, 0, 256, 132};
		RECT_FIXED lightl_dst = {
			FIXED_DEC(-175,1) - fx,
			FIXED_DEC(-80,1) - fy,
			FIXED_DEC(195,1),
			FIXED_DEC(103,1)
		};
		
		RECT lightr_src = {0, 132, 256, 124};
		RECT_FIXED lightr_dst = {
			FIXED_DEC(98,1) - fx,
			FIXED_DEC(-64,1) - fy,
			FIXED_DEC(198,1),
			FIXED_DEC(95,1)
		};
		
		u8 win_r = (((fixed_t)this->win_r * this->win_time) >> FIXED_SHIFT) / 6;
		u8 win_g = (((fixed_t)this->win_g * this->win_time) >> FIXED_SHIFT) / 6;
		u8 win_b = (((fixed_t)this->win_b * this->win_time) >> FIXED_SHIFT) / 6;
		
		Stage_DrawTexCol(&this->tex_back1, &lightl_src, &lightl_dst, stage.camera.bzoom, win_r, win_g, win_b);
		Stage_DrawTexCol(&this->tex_back1, &lightr_src, &lightr_dst, stage.camera.bzoom, win_r, win_g, win_b);
	}
	
	//Draw buildings
	RECT building_src = {0, 0, 255, 128};
	RECT_FIXED building_dst = {
		FIXED_DEC(-195,1) - fx,
		FIXED_DEC(-120,1) - fy,
		FIXED_DEC(240,1),
		FIXED_DEC(120,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &building_src, &building_dst, stage.camera.bzoom);
	building_dst.x += building_dst.w;
	building_src.y += building_src.h;
	Stage_DrawTex(&this->tex_back0, &building_src, &building_dst, stage.camera.bzoom);
	
	RECT building_fillsrc = {0, 255, 1, 0};
	RECT building_fill = {0, SCREEN_HEIGHT * 3 / 7, SCREEN_WIDTH, SCREEN_HEIGHT * 4 / 7};
	Gfx_DrawTex(&this->tex_back0, &building_fillsrc, &building_fill);
	
	//Draw sky
	fx = stage.camera.x >> 3;
	fy = stage.camera.y >> 3;
	
	RECT sky_src = {0, 0, 255, 128};
	RECT_FIXED sky_dst = {
		FIXED_DEC(-166,1) - FIXED_DEC(SCREEN_WIDEOADD,2) - fx,
		FIXED_DEC(-117,1) - FIXED_DEC(SCREEN_WIDEOADD,4) - fy,
		FIXED_DEC(172,1) + FIXED_DEC(SCREEN_WIDEOADD,1),
		FIXED_DEC(110,1) + FIXED_DEC(SCREEN_WIDEOADD,2)
	};
	
	Stage_DrawTex(&this->tex_back5, &sky_src, &sky_dst, stage.camera.bzoom);
	sky_dst.x += sky_dst.w;
	sky_src.y += sky_src.h;
	Stage_DrawTex(&this->tex_back5, &sky_src, &sky_dst, stage.camera.bzoom);
}

void Back_Week3_Free(StageBack *back)
{
	Back_Week3 *this = (Back_Week3*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week3_New(void)
{
	//Allocate background structure
	Back_Week3 *this = (Back_Week3*)Mem_Alloc(sizeof(Back_Week3));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week3_DrawBG;
	this->back.free = Back_Week3_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK3\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Gfx_LoadTex(&this->tex_back4, Archive_Find(arc_back, "back4.tim"), 0);
	Gfx_LoadTex(&this->tex_back5, Archive_Find(arc_back, "back5.tim"), 0);
	Mem_Free(arc_back);
	
	//Initialize window state
	this->win_time = -1;
	
	//Initialize train state
	this->train_x = TRAIN_END_X;
	this->train_timer = RandomRange(TRAIN_TIME_A, TRAIN_TIME_B);
	
	return (StageBack*)this;
}
