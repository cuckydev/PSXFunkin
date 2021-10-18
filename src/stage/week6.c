/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week6.h"

#include "../stage.h"
#include "../archive.h"
#include "../mem.h"
#include "../mutil.h"
#include "../timer.h"

//Week 6 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //Background
	Gfx_Tex tex_back1; //Trees
	Gfx_Tex tex_back2; //Freaks
	
	//Freaks state
	Animatable freaks_animatable;
	u8 freaks_frame;
} Back_Week6;

//Freaks animation and rects
static const CharFrame freaks_frame[] = {
	{0, {  0,   0,  83,  79}, { 17,  79}}, //normal 0
	{0, { 84,   0,  83,  76}, { 18,  76}}, //normal 1
	{0, {168,   0,  84,  82}, { 17,  82}}, //normal 2
	{0, {  0,  80,  83,  81}, { 17,  81}}, //normal 3
	
	{0, { 84,  77,  83,  79}, { 17,  79}}, //disuaded 0
	{0, {168,  83,  83,  76}, { 18,  76}}, //disuaded 1
	{0, {  0, 162,  84,  82}, { 17,  82}}, //disuaded 2
	{0, { 85, 157,  83,  81}, { 17,  81}}, //disuaded 3
};

static const Animation freaks_anim[] = {
	{2, (const u8[]){1, 0, 0, 3, ASCR_BACK, 1}},
	{2, (const u8[]){2, 3, 3, 0, ASCR_BACK, 1}},
};

//Freaks functions
void Week6_Freaks_SetFrame(void *user, u8 frame)
{
	Back_Week6 *this = (Back_Week6*)user;
	this->freaks_frame = frame;
}

void Week6_Freaks_Draw(Back_Week6 *this, fixed_t x, fixed_t y, boolean flip)
{
	//Draw character
	static const u8 frame_map[4][2][2] = {
		{{0, 3}, {4, 7}},
		{{1, 2}, {5, 6}},
		{{2, 1}, {6, 5}},
		{{3, 0}, {7, 4}},
	};
	
	const CharFrame *cframe = &freaks_frame[frame_map[this->freaks_frame][stage.stage_id == StageId_6_2][flip]];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_back2, &src, &dst, stage.camera.bzoom);
}

//Week 6 background functions
void Back_Week6_DrawBG(StageBack *back)
{
	Back_Week6 *this = (Back_Week6*)back;
	
	fixed_t fx, fy;
	
	//Animate and draw freaks
	fx = (stage.camera.x << 2) / 5;
	fy = (stage.camera.y << 2) / 5;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		switch (stage.song_step & 7)
		{
			case 0:
				Animatable_SetAnim(&this->freaks_animatable, 0);
				break;
			case 4:
				Animatable_SetAnim(&this->freaks_animatable, 1);
				break;
		}
	}
	Animatable_Animate(&this->freaks_animatable, (void*)this, Week6_Freaks_SetFrame);
	
	Week6_Freaks_Draw(this, FIXED_DEC(-110,1) - fx, FIXED_DEC(44,1) - fy, false);
	Week6_Freaks_Draw(this,  FIXED_DEC(-20,1) - fx, FIXED_DEC(44,1) - fy, true);
	Week6_Freaks_Draw(this,   FIXED_DEC(70,1) - fx, FIXED_DEC(44,1) - fy, false);
	
	//Draw foreground trees
	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;
	
	static const struct Back_Week6_FGTree
	{
		RECT src;
		fixed_t x, y;
		fixed_t off[4];
	} fg_tree[] = {
		{{  0, 0, 100, 82}, FIXED_DEC(-116,1), FIXED_DEC(-88,1), {FIXED_DEC(32,10), FIXED_DEC(34,10), FIXED_DEC(28,10), FIXED_DEC(30,10)}},
		{{101, 0,  99, 80},    FIXED_DEC(0,1), FIXED_DEC(-84,1), {FIXED_DEC(31,10), FIXED_DEC(30,10), FIXED_DEC(29,10), FIXED_DEC(32,10)}},
	};
	
	const struct Back_Week6_FGTree *fg_tree_p = fg_tree;
	for (size_t i = 0; i < COUNT_OF(fg_tree); i++, fg_tree_p++)
	{
		//Get distorted points
		POINT_FIXED treep[4] = {
			{fg_tree_p->x,                                 fg_tree_p->y},
			{fg_tree_p->x + FIXED_DEC(fg_tree_p->src.w,1), fg_tree_p->y},
			{fg_tree_p->x,                                 fg_tree_p->y + FIXED_DEC(fg_tree_p->src.h,1)},
			{fg_tree_p->x + FIXED_DEC(fg_tree_p->src.w,1), fg_tree_p->y + FIXED_DEC(fg_tree_p->src.h,1)},
		};
		for (int j = 0; j < 4; j++)
		{
			treep[j].x += ((MUtil_Cos(FIXED_MUL(animf_count, fg_tree_p->off[j])) * FIXED_DEC(3,1)) >> 8) - fx;
			treep[j].y += ((MUtil_Sin(FIXED_MUL(animf_count, fg_tree_p->off[j])) * FIXED_DEC(3,1)) >> 8) - fy;
		}
		
		Stage_DrawTexArb(&this->tex_back1, &fg_tree_p->src, &treep[0], &treep[1], &treep[2], &treep[3], stage.camera.bzoom);
	}
	
	//Draw background trees
	RECT bg_tree_l_src = {0, 83, 62, 45};
	RECT_FIXED bg_tree_l_dst = {
		FIXED_DEC(-106,1) - fx,
		FIXED_DEC(-26,1) - fy,
		FIXED_DEC(62,1),
		FIXED_DEC(45,1)
	};
	
	RECT bg_tree_r_src = {63, 83, 61, 50};
	RECT_FIXED bg_tree_r_dst = {
		FIXED_DEC(44,1) - fx,
		FIXED_DEC(-26,1) - fy,
		FIXED_DEC(61,1),
		FIXED_DEC(50,1)
	};
	
	Stage_DrawTex(&this->tex_back1, &bg_tree_l_src, &bg_tree_l_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back1, &bg_tree_r_src, &bg_tree_r_dst, stage.camera.bzoom);
	
	//Draw school
	fx = stage.camera.x >> 3;
	fy = stage.camera.y >> 3;
	
	RECT school_src = {0, 0, 255, 75};
	RECT_FIXED school_dst = {
		FIXED_DEC(-128,1) - fx,
		FIXED_DEC(-56,1) - fy,
		FIXED_DEC(255,1),
		FIXED_DEC(75,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &school_src, &school_dst, stage.camera.bzoom);
	
	//Draw street
	//fx = stage.camera.x >> 3;
	//fy = stage.camera.y >> 3;
	
	POINT_FIXED street_d0 = {
		FIXED_DEC(-128,1) - fx,
		FIXED_DEC(19,1) - FIXED_DEC(1,1) - fy,
	};
	POINT_FIXED street_d1 = {
		FIXED_DEC(-128,1) + FIXED_DEC(255,1) - fx,
		FIXED_DEC(19,1) - FIXED_DEC(1,1) - fy,
	};
	
	fx = stage.camera.x * 3 / 2;
	fy = stage.camera.y * 3 / 2;
	
	POINT_FIXED street_d2 = {
		FIXED_DEC(-128,1) - fx,
		FIXED_DEC(19,1) + FIXED_DEC(58,1) - fy,
	};
	POINT_FIXED street_d3 = {
		FIXED_DEC(-128,1) + FIXED_DEC(255,1) - fx,
		FIXED_DEC(19,1) + FIXED_DEC(58,1) - fy,
	};
	
	RECT street_src = {0, 75, 255, 54};
	
	Stage_DrawTexArb(&this->tex_back0, &street_src, &street_d0, &street_d1, &street_d2, &street_d3, stage.camera.bzoom);
	
	//Draw sky
	fx = stage.camera.x >> 4;
	fy = stage.camera.y >> 4;
	
	RECT sky_src = {0, 130, 255, 125};
	RECT_FIXED sky_dst = {
		FIXED_DEC(-128,1) - fx,
		FIXED_DEC(-72,1) - fy,
		FIXED_DEC(255,1),
		FIXED_DEC(125,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &sky_src, &sky_dst, stage.camera.bzoom);
}

static fixed_t week6_back_paraly[] = {
	FIXED_DEC(15,100),
	FIXED_DEC(15,100),
	FIXED_DEC(15,100),
	FIXED_DEC(15,100),
	FIXED_DEC(7,10),
	FIXED_DEC(13,10),
};

static fixed_t week6_back_warpx[] = {
	FIXED_DEC(5,1),
	FIXED_DEC(5,1),
	FIXED_DEC(5,1),
	FIXED_DEC(4,1),
	FIXED_DEC(3,1),
	FIXED_DEC(3,1),
};

static fixed_t week6_back_warpy[] = {
	FIXED_DEC(25,10),
	FIXED_DEC(20,10),
	FIXED_DEC(15,10),
	FIXED_DEC(10,10),
	FIXED_DEC(0,10),
	FIXED_DEC(0,10),
};

static s32 Back_Week6_GetX(int x, int y)
{
	return ((fixed_t)x << (FIXED_SHIFT + 5)) + FIXED_DEC(-128,1) - FIXED_MUL(stage.camera.x, week6_back_paraly[y]) + ((MUtil_Cos((animf_count << 2) + ((x + y) << 5)) * week6_back_warpx[y]) >> 8);
}

static s32 Back_Week6_GetY(int x, int y)
{
	return ((fixed_t)y << (FIXED_SHIFT + 5)) + FIXED_DEC(-86,1) - FIXED_MUL(stage.camera.y, week6_back_paraly[y]) + ((MUtil_Sin((animf_count << 2) + ((x + y) << 5)) * week6_back_warpy[y]) >> 8);
}

void Back_Week6_DrawBG3(StageBack *back)
{
	Back_Week6 *this = (Back_Week6*)back;
	
	//Get quad points
	POINT_FIXED back_dst[6][9];
	for (int y = 0; y < 6; y++)
	{
		for (int x = 0; x < 9; x++)
		{
			back_dst[y][x].x = Back_Week6_GetX(x, y);
			back_dst[y][x].y = Back_Week6_GetY(x, y);
		}
	}
	
	//Draw 32x32 quads of the background
	for (int y = 0; y < 5; y++)
	{
		RECT back_src = {0, y * 32, 32, 32};
		for (int x = 0; x < 8; x++)
		{
			//Draw quad and increment source rect
			Stage_DrawTexArb(&this->tex_back0, &back_src, &back_dst[y][x], &back_dst[y][x + 1], &back_dst[y + 1][x], &back_dst[y + 1][x + 1], stage.camera.bzoom);
			if ((back_src.x += 32) >= 0xE0)
				back_src.w--;
		}
	}
}

void Back_Week6_Free(StageBack *back)
{
	Back_Week6 *this = (Back_Week6*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week6_New(void)
{
	//Allocate background structure
	Back_Week6 *this = (Back_Week6*)Mem_Alloc(sizeof(Back_Week6));
	if (this == NULL)
		return NULL;
	
	if (stage.stage_id != StageId_6_3)
	{
		//Set background functions
		this->back.draw_fg = NULL;
		this->back.draw_md = NULL;
		this->back.draw_bg = Back_Week6_DrawBG;
		this->back.free = Back_Week6_Free;
		
		//Load background textures
		IO_Data arc_back = IO_Read("\\WEEK6\\BACK.ARC;1");
		Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
		Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
		Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
		Mem_Free(arc_back);
		
		//Initialize freaks state
		Animatable_Init(&this->freaks_animatable, freaks_anim);
		Animatable_SetAnim(&this->freaks_animatable, 0);
	}
	else
	{
		//Set background functions
		this->back.draw_fg = NULL;
		this->back.draw_md = NULL;
		this->back.draw_bg = Back_Week6_DrawBG3;
		this->back.free = Back_Week6_Free;
		
		//Load background texture
		Gfx_LoadTex(&this->tex_back0, IO_Read("\\WEEK6\\BACK3.TIM;1"), GFX_LOADTEX_FREE);
	}
	
	return (StageBack*)this;
}
