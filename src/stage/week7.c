#include "week7.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"

//Week 7 background functions
void Back_Week7_DrawFG(StageBack *back)
{
	Back_Week7 *this = (Back_Week7*)back;
	
	fixed_t fx, fy;
}

void Back_Week7_DrawBG(StageBack *back)
{
	Back_Week7 *this = (Back_Week7*)back;
	
	fixed_t fx, fy;
	
	//Draw foreground
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT fg_src = {0, 0, 255, 128};
	RECT_FIXED fg_dst = {
		FIXED_DEC(-340,1) - fx,
		FIXED_DEC(18,1) - fy,
		FIXED_DEC(340,1),
		FIXED_DEC(170,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &fg_src, &fg_dst, stage.camera.bzoom);
	fg_dst.x += fg_dst.w;
	fg_src.y += 128;
	Stage_DrawTex(&this->tex_back0, &fg_src, &fg_dst, stage.camera.bzoom);
	
	//Draw sniper
	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;
	
	RECT snipe_src = {0, 0, 128, 256};
	RECT_FIXED snipe_dst = {
		FIXED_DEC(-190,1) - fx,
		FIXED_DEC(-90,1) - fy,
		FIXED_DEC(90,1),
		FIXED_DEC(180,1),
	};
	
	fixed_t snipe_bop;
	if ((stage.song_step & 0x3) == 0)
		snipe_bop = FIXED_UNIT - (stage.note_scroll & FIXED_LAND);
	else
		snipe_bop = 0;
	
	snipe_dst.x -= snipe_bop << 1;
	snipe_dst.y += snipe_bop << 2;
	snipe_dst.w += snipe_bop << 2;
	snipe_dst.h -= snipe_bop << 2;
	
	Stage_DrawTex(&this->tex_back2, &snipe_src, &snipe_dst, stage.camera.bzoom);
	
	//Draw ruins
	fx = stage.camera.x >> 2;
	fy = stage.camera.y >> 2;
	
	RECT ruinsf_src = {0, 0, 256, 72};
	RECT_FIXED ruinsf_dst = {
		FIXED_DEC(-240,1) - fx,
		FIXED_DEC(-70,1) - fy,
		FIXED_DEC(480,1),
		FIXED_DEC(135,1)
	};
	
	Stage_DrawTex(&this->tex_back1, &ruinsf_src, &ruinsf_dst, stage.camera.bzoom);
	
	fx = stage.camera.x / 6;
	fy = stage.camera.y / 6;
	
	RECT ruinsb_src = {0, 72, 256, 44};
	RECT_FIXED ruinsb_dst = {
		FIXED_DEC(-240,1) - fx,
		FIXED_DEC(-60,1) - fy,
		FIXED_DEC(480,1),
		FIXED_DEC(135,1)
	};
	
	Stage_DrawTex(&this->tex_back1, &ruinsb_src, &ruinsb_dst, stage.camera.bzoom);
	
	//Draw mountains
	fx = stage.camera.x >> 3;
	fy = stage.camera.y >> 3;
	
	RECT mountain_src = {0, 0, 255, 128};
	RECT_FIXED mountain_dst = {
		FIXED_DEC(-260,1) - fx,
		FIXED_DEC(-110,1) - fy,
		FIXED_DEC(260,1),
		FIXED_DEC(130,1)
	};
	
	Stage_DrawTex(&this->tex_back3, &mountain_src, &mountain_dst, stage.camera.bzoom);
	mountain_dst.x += mountain_dst.w;
	mountain_src.y += 128;
	Stage_DrawTex(&this->tex_back3, &mountain_src, &mountain_dst, stage.camera.bzoom);
}

void Back_Week7_Free(StageBack *back)
{
	Back_Week7 *this = (Back_Week7*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week7_New()
{
	//Allocate background structure
	Back_Week7 *this = (Back_Week7*)Mem_Alloc(sizeof(Back_Week7));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_Week7_DrawFG;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week7_DrawBG;
	this->back.free = Back_Week7_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK7\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Mem_Free(arc_back);
	
	//Use sky coloured background
	Gfx_SetClear(245, 202, 81);
	
	return (StageBack*)this;
}
