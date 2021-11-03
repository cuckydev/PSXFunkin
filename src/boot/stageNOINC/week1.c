/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week1.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"

//Week 1 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //Stage and back
	Gfx_Tex tex_back1; //Curtains
} Back_Week1;

//Week 1 background functions
void Back_Week1_DrawBG(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
	
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
	
	Stage_DrawTex(&this->tex_back1, &curtainl_src, &curtainl_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back1, &curtainr_src, &curtainr_dst, stage.camera.bzoom);
	
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
	
	Stage_DrawTexArb(&this->tex_back0, &stage_src, &stage_d0, &stage_d1, &stage_d2, &stage_d3, stage.camera.bzoom);
	
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
	
	Stage_DrawTex(&this->tex_back0, &backl_src, &backl_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back0, &backr_src, &backr_dst, stage.camera.bzoom);
	Gfx_DrawTex(&this->tex_back0, &backf_src, &backf_dst);
}

void Back_Week1_Free(StageBack *back)
{
	Back_Week1 *this = (Back_Week1*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week1_New(void)
{
	//Allocate background structure
	Back_Week1 *this = (Back_Week1*)Mem_Alloc(sizeof(Back_Week1));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week1_DrawBG;
	this->back.free = Back_Week1_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK1\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
