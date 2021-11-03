/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week1.h"

#include "boot/stage.h"
#include "boot/archive.h"
#include "boot/mem.h"

//Week 1 assets
static const u8 week1_arc_back[] = {
	#include "iso/week1/back.arc.h"
};

#include "character/bf.c"
#include "character/dad.c"
#define CHAR_GF_TUTORIAL
#include "character/gf.c"
#undef CHAR_GF_TUTORIAL

//Week 1 textures
static Gfx_Tex week1_tex_back0; //Stage and back
static Gfx_Tex week1_tex_back1; //Curtains

//Week 1 background functions
void Week1_Load(void)
{
	//Load background textures
	Gfx_LoadTex(&week1_tex_back0, Archive_Find((IO_Data)week1_arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&week1_tex_back1, Archive_Find((IO_Data)week1_arc_back, "back1.tim"), 0);
	
	//Load characters
	stage.player = Char_BF_New(FIXED_DEC(60,1), FIXED_DEC(100,1));
	if (stage.stage_id == StageId_1_4)
	{
		//GF as opponent
		stage.opponent = Char_GF_New(FIXED_DEC(0,1), FIXED_DEC(-10,1));
		stage.gf = NULL;
	}
	else
	{
		//Dad as opponent
		stage.opponent = Char_Dad_New(FIXED_DEC(-120,1), FIXED_DEC(100,1));
		stage.gf = Char_GF_New(FIXED_DEC(0,1), FIXED_DEC(-10,1));
	}
}

void Week1_DrawBG()
{
	fixed_t fx, fy;
	
	//Draw curtains
	fx = (stage.camera.x * 5) >> 2;
	fy = (stage.camera.y * 5) >> 2;
	
	RECT curtainl_src = {0, 0, 107, 221};
	RECT_FIXED curtainl_dst = {
		FIXED_DEC(-250,1) - FIXED_DEC(SCREEN_WIDEOADD,2) - fx,
		FIXED_DEC(-150,1) - fy,
		FIXED_DEC(107,1),
		FIXED_DEC(221,1)
	};
	RECT curtainr_src = {122, 0, 134, 256};
	RECT_FIXED curtainr_dst = {
		FIXED_DEC(110,1) + FIXED_DEC(SCREEN_WIDEOADD,2) - fx,
		FIXED_DEC(-150,1) - fy,
		FIXED_DEC(134,1),
		FIXED_DEC(256,1)
	};
	
	Stage_DrawTex(&week1_tex_back1, &curtainl_src, &curtainl_dst, stage.camera.bzoom);
	Stage_DrawTex(&week1_tex_back1, &curtainr_src, &curtainr_dst, stage.camera.bzoom);
	
	//Draw stage
	fx = stage.camera.x * 3 / 2;
	fy = stage.camera.y * 3 / 2;
	
	POINT_FIXED stage_d2 = {
		FIXED_DEC(-230,1) - fx,
		FIXED_DEC(50,1) + FIXED_DEC(123,1) - fy,
	};
	POINT_FIXED stage_d3 = {
		FIXED_DEC(-230,1) + FIXED_DEC(410,1) - fx,
		FIXED_DEC(50,1) + FIXED_DEC(123,1) - fy,
	};
	
	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;
	
	POINT_FIXED stage_d0 = {
		FIXED_DEC(-230,1) - fx,
		FIXED_DEC(50,1) - fy,
	};
	POINT_FIXED stage_d1 = {
		FIXED_DEC(-230,1) + FIXED_DEC(410,1) - fx,
		FIXED_DEC(50,1) - fy,
	};
	
	RECT stage_src = {0, 0, 255, 59};
	
	Stage_DrawTexArb(&week1_tex_back0, &stage_src, &stage_d0, &stage_d1, &stage_d2, &stage_d3, stage.camera.bzoom);
	
	//Draw back
	//fx = stage.camera.x * 2 / 3;
	//fy = stage.camera.y * 2 / 3;
	
	RECT backl_src = {0, 59, 121, 105};
	RECT_FIXED backl_dst = {
		FIXED_DEC(-190,1) - fx,
		FIXED_DEC(-100,1) - fy,
		FIXED_DEC(121,1),
		FIXED_DEC(105,1)
	};
	RECT backr_src = {121, 59, 136, 120};
	RECT_FIXED backr_dst = {
		FIXED_DEC(60,1) - fx,
		FIXED_DEC(-110,1) - fy,
		FIXED_DEC(136,1),
		FIXED_DEC(120,1)
	};
	RECT backf_src = {0, 59, 1, 1};
	RECT backf_dst = {
		0,
		0,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
	};
	
	Stage_DrawTex(&week1_tex_back0, &backl_src, &backl_dst, stage.camera.bzoom);
	Stage_DrawTex(&week1_tex_back0, &backr_src, &backr_dst, stage.camera.bzoom);
	Gfx_DrawTex(&week1_tex_back0, &backf_src, &backf_dst);
}

void Week1_SetPtr(void)
{
	//Set pointers
	stageoverlay_load = Week1_Load;
	stageoverlay_drawbg = Week1_DrawBG;
	stageoverlay_drawmd = NULL;
	stageoverlay_drawfg = NULL;
	stageoverlay_free = NULL;
	
	//Set stage information
	stageoverlay_chartfmt = "\\WEEK1\\%d%c.CHT;1";
}
