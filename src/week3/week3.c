/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week3.h"

#include "boot/stage.h"
#include "boot/archive.h"
#include "boot/main.h"
#include "boot/mem.h"

//Charts
static u8 week3_cht_pico_easy[] = {
	#include "iso/chart/pico-easy.json.cht.h"
};
static u8 week3_cht_pico_normal[] = {
	#include "iso/chart/pico.json.cht.h"
};
static u8 week3_cht_pico_hard[] = {
	#include "iso/chart/pico-hard.json.cht.h"
};

static u8 week3_cht_philly_easy[] = {
	#include "iso/chart/philly-easy.json.cht.h"
};
static u8 week3_cht_philly_normal[] = {
	#include "iso/chart/philly.json.cht.h"
};
static u8 week3_cht_philly_hard[] = {
	#include "iso/chart/philly-hard.json.cht.h"
};

static u8 week3_cht_blammed_easy[] = {
	#include "iso/chart/blammed-easy.json.cht.h"
};
static u8 week3_cht_blammed_normal[] = {
	#include "iso/chart/blammed.json.cht.h"
};
static u8 week3_cht_blammed_hard[] = {
	#include "iso/chart/blammed-hard.json.cht.h"
};

static IO_Data week3_cht[][3] = {
	{
		(IO_Data)week3_cht_pico_easy,
		(IO_Data)week3_cht_pico_normal,
		(IO_Data)week3_cht_pico_hard,
	},
	{
		(IO_Data)week3_cht_philly_easy,
		(IO_Data)week3_cht_philly_normal,
		(IO_Data)week3_cht_philly_hard,
	},
	{
		(IO_Data)week3_cht_blammed_easy,
		(IO_Data)week3_cht_blammed_normal,
		(IO_Data)week3_cht_blammed_hard,
	},
};

//Characters
//Boyfriend
#include "character/bf.c"

//Pico
#include "character/pico.c"

//Girlfriend
#include "character/gf.c"

static fixed_t Char_GF_GetParallax(Char_GF *this)
{
	(void)this;
	return FIXED_UNIT;
}

//Week 3 textures
static Gfx_Tex week3_tex_back0; //Buildings
static Gfx_Tex week3_tex_back1; //Lights
static Gfx_Tex week3_tex_back2; //Rooftop
static Gfx_Tex week3_tex_back3; //Background train arc
static Gfx_Tex week3_tex_back4; //Train
static Gfx_Tex week3_tex_back5; //Sky

//Window state
static u8 week3_win_r, week3_win_g, week3_win_b;
static fixed_t week3_win_time;

//Train state
#define TRAIN_START_X FIXED_DEC(500,1)
#define TRAIN_END_X    FIXED_DEC(-8000,1)
#define TRAIN_TIME_A 6
#define TRAIN_TIME_B 14

static fixed_t week3_train_x;
static u8 week3_train_timer;

//Week 3 background functions
static void Week3_Load(void)
{
	//Load assets
	Gfx_LoadTex(&stage.tex_hud0, Overlay_DataRead(), 0); //hud0.tim
	Gfx_LoadTex(&stage.tex_hud1, Overlay_DataRead(), 0); //hud1.tim
	
	Gfx_LoadTex(&week3_tex_back0, Overlay_DataRead(), 0); //back0.tim
	Gfx_LoadTex(&week3_tex_back1, Overlay_DataRead(), 0); //back1.tim
	Gfx_LoadTex(&week3_tex_back2, Overlay_DataRead(), 0); //back2.tim
	Gfx_LoadTex(&week3_tex_back3, Overlay_DataRead(), 0); //back3.tim
	Gfx_LoadTex(&week3_tex_back4, Overlay_DataRead(), 0); //back4.tim
	Gfx_LoadTex(&week3_tex_back5, Overlay_DataRead(), 0); //back5.tim
	
	Overlay_DataFree();
	
	//Load characters
	stage.player = Char_BF_New(FIXED_DEC(56,1), FIXED_DEC(85,1));
	stage.opponent = Char_Pico_New(FIXED_DEC(-105,1), FIXED_DEC(85,1));
	stage.gf = Char_GF_New(FIXED_DEC(0,1), FIXED_DEC(-15,1));
	
	//Initialize window state
	week3_win_time = -1;
	
	//Initialize train state
	week3_train_x = TRAIN_END_X;
	week3_train_timer = RandomRange(TRAIN_TIME_A, TRAIN_TIME_B);
}

static void Week3_DrawBG()
{
	fixed_t fx, fy;
	
	//Update window
	static const u8 win_cols[][3] = {
		{ 49, 162, 253},
		{ 49, 253, 140},
		{251,  51, 245},
		{253,  69,  49},
		{251, 166,  51},
	};
	
	if (week3_win_time > 0)
	{
		week3_win_time -= timer_dt;
		if (week3_win_time < 0)
			week3_win_time = 0;
	}
	if (stage.note_scroll >= 0 && (stage.flag & STAGE_FLAG_JUST_STEP) && (stage.song_step & 0xF) == 0)
	{
		const u8 *win_col = win_cols[RandomRange(0, COUNT_OF(win_cols) - 1)];
		week3_win_r = win_col[0];
		week3_win_g = win_col[1];
		week3_win_b = win_col[2];
		week3_win_time = FIXED_DEC(3,1);
	}
	
	//Draw rooftop
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	static const struct Back_Week3_RoofPiece
	{
		RECT src;
		fixed_t scale;
	} roof_piece[] = {
		{{  0, 0,  16, 255}, FIXED_MUL(FIXED_DEC(3,1) * 7 / 10, FIXED_UNIT + FIXED_DEC(SCREEN_WIDEOADD,2) * 10 / 336)},
		{{ 16, 0,  55, 255}, FIXED_DEC(1,1) * 9 / 10},
		{{ 71, 0, 128, 255}, FIXED_DEC(265,100) * 7 / 10},
		{{199, 0,  55, 255}, FIXED_DEC(1,1) * 9 / 10},
		{{255, 0,   0, 255}, FIXED_DEC(16,1) + FIXED_DEC(SCREEN_WIDEOADD2,1)}
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
		Stage_DrawTex(&week3_tex_back2, &roof_p->src, &roof_dst, stage.camera.bzoom);
		roof_dst.x += roof_dst.w;
	}
	
	RECT roof_fillsrc = {0, 254, 1, 0};
	RECT roof_fill = {0, SCREEN_HEIGHT * 2 / 3, SCREEN_WIDTH, SCREEN_HEIGHT * 1 / 3};
	Gfx_DrawTex(&week3_tex_back2, &roof_fillsrc, &roof_fill);
	
	//Move train
	if (week3_train_x <= TRAIN_END_X)
	{
		//Reset train
		if ((stage.flag & STAGE_FLAG_JUST_STEP) && (stage.song_step & 0xF) == 0)
		{
			if (--week3_train_timer == 0)
			{
				week3_train_x = TRAIN_START_X;
				week3_train_timer = RandomRange(TRAIN_TIME_A, TRAIN_TIME_B);
			}
		}
	}
	else
	{
		//Move train to end position
		week3_train_x  -= timer_dt * 2000;
		
		//Draw train
		RECT train_src = {0, 0, 255, 255};
		RECT_FIXED train_dst = {
			week3_train_x - fx,
			FIXED_DEC(-65,1) - fy,
			FIXED_DEC(284,1),
			FIXED_DEC(119,1)
		};
		
		for (int i = 0; i < 24; i++, train_dst.x += train_dst.w)
		{
			if (train_dst.x >= (SCREEN_WIDTH2 << FIXED_SHIFT) || train_dst.x <= -(train_dst.w + (SCREEN_WIDTH2 << FIXED_SHIFT)))
				continue;
			Stage_DrawTex(&week3_tex_back4, &train_src, &train_dst, stage.camera.bzoom);
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
	
	Stage_DrawTex(&week3_tex_back3, &arcl_src, &arcl_dst, stage.camera.bzoom);
	Stage_DrawTex(&week3_tex_back3, &arcr_src, &arcr_dst, stage.camera.bzoom);
	
	//Draw lights
	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;
	
	if (week3_win_time >= 0)
	{
		RECT lightl_src = {0, 0, 255, 132};
		RECT_FIXED lightl_dst = {
			FIXED_DEC(-175,1) - fx,
			FIXED_DEC(-80,1) - fy,
			FIXED_DEC(195,1),
			FIXED_DEC(103,1)
		};
		
		RECT lightr_src = {0, 132, 255, 123};
		RECT_FIXED lightr_dst = {
			FIXED_DEC(98,1) - fx,
			FIXED_DEC(-64,1) - fy,
			FIXED_DEC(198,1),
			FIXED_DEC(95,1)
		};
		
		u8 win_r = (((fixed_t)week3_win_r * week3_win_time) >> FIXED_SHIFT) / 6;
		u8 win_g = (((fixed_t)week3_win_g * week3_win_time) >> FIXED_SHIFT) / 6;
		u8 win_b = (((fixed_t)week3_win_b * week3_win_time) >> FIXED_SHIFT) / 6;
		
		Stage_DrawTexCol(&week3_tex_back1, &lightl_src, &lightl_dst, stage.camera.bzoom, win_r, win_g, win_b);
		Stage_DrawTexCol(&week3_tex_back1, &lightr_src, &lightr_dst, stage.camera.bzoom, win_r, win_g, win_b);
	}
	
	//Draw buildings
	RECT building_src = {0, 0, 255, 127};
	RECT_FIXED building_dst = {
		FIXED_DEC(-195,1) - fx,
		FIXED_DEC(-120,1) - fy,
		FIXED_DEC(240,1),
		FIXED_DEC(120,1)
	};
	
	Stage_DrawTex(&week3_tex_back0, &building_src, &building_dst, stage.camera.bzoom);
	building_dst.x += building_dst.w;
	building_src.y += 128;
	Stage_DrawTex(&week3_tex_back0, &building_src, &building_dst, stage.camera.bzoom);
	
	RECT building_fillsrc = {0, 255, 1, 0};
	RECT building_fill = {0, SCREEN_HEIGHT * 3 / 7, SCREEN_WIDTH, SCREEN_HEIGHT * 4 / 7};
	Gfx_DrawTex(&week3_tex_back0, &building_fillsrc, &building_fill);
	
	//Draw sky
	fx = stage.camera.x >> 3;
	fy = stage.camera.y >> 3;
	
	RECT sky_src = {0, 0, 255, 127};
	RECT_FIXED sky_dst = {
		FIXED_DEC(-166,1) - FIXED_DEC(SCREEN_WIDEOADD,2) - fx,
		FIXED_DEC(-117,1) - FIXED_DEC(SCREEN_WIDEOADD,4) - fy,
		FIXED_DEC(172,1) + FIXED_DEC(SCREEN_WIDEOADD,1),
		FIXED_DEC(110,1) + FIXED_DEC(SCREEN_WIDEOADD,2)
	};
	
	Stage_DrawTex(&week3_tex_back5, &sky_src, &sky_dst, stage.camera.bzoom);
	sky_dst.x += sky_dst.w;
	sky_src.y += 128;
	Stage_DrawTex(&week3_tex_back5, &sky_src, &sky_dst, stage.camera.bzoom);
}

static IO_Data Week3_GetChart(void)
{
	return week3_cht[stage.stage_id - StageId_3_1][stage.stage_diff];
}

static boolean Week3_LoadScreen(void)
{
	return false;
}

static boolean Week3_NextStage(void)
{
	switch (stage.stage_id)
	{
		case StageId_3_1: //Pico
			stage.stage_id = StageId_3_2;
			return true;
		case StageId_3_2: //Philly Nice
			stage.stage_id = StageId_3_3;
			return true;
		case StageId_3_3: //Blammed
			return false;
		default:
			return false;
	}
}

void Week3_SetPtr(void)
{
	//Set pointers
	stageoverlay_load = Week3_Load;
	stageoverlay_tick = NULL;
	stageoverlay_drawbg = Week3_DrawBG;
	stageoverlay_drawmd = NULL;
	stageoverlay_drawfg = NULL;
	stageoverlay_free = NULL;
	stageoverlay_getchart = Week3_GetChart;
	stageoverlay_loadscreen = Week3_LoadScreen;
	stageoverlay_nextstage = Week3_NextStage;
}
