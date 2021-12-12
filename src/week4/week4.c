/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week4.h"

#include "boot/stage.h"
#include "boot/archive.h"
#include "boot/main.h"
#include "boot/mem.h"

//Charts
static u8 week4_cht_satin_panties_easy[] = {
	#include "iso/chart/satin-panties-easy.json.cht.h"
};
static u8 week4_cht_satin_panties_normal[] = {
	#include "iso/chart/satin-panties.json.cht.h"
};
static u8 week4_cht_satin_panties_hard[] = {
	#include "iso/chart/satin-panties-hard.json.cht.h"
};

static u8 week4_cht_high_easy[] = {
	#include "iso/chart/high-easy.json.cht.h"
};
static u8 week4_cht_high_normal[] = {
	#include "iso/chart/high.json.cht.h"
};
static u8 week4_cht_high_hard[] = {
	#include "iso/chart/high-hard.json.cht.h"
};

static u8 week4_cht_milf_easy[] = {
	#include "iso/chart/milf-easy.json.cht.h"
};
static u8 week4_cht_milf_normal[] = {
	#include "iso/chart/milf.json.cht.h"
};
static u8 week4_cht_milf_hard[] = {
	#include "iso/chart/milf-hard.json.cht.h"
};

static u8 week4_cht_test[] = {
	#include "iso/chart/test.json.cht.h"
};

static IO_Data week4_cht[][3] = {
	{
		(IO_Data)week4_cht_satin_panties_easy,
		(IO_Data)week4_cht_satin_panties_normal,
		(IO_Data)week4_cht_satin_panties_hard,
	},
	{
		(IO_Data)week4_cht_high_easy,
		(IO_Data)week4_cht_high_normal,
		(IO_Data)week4_cht_high_hard,
	},
	{
		(IO_Data)week4_cht_milf_easy,
		(IO_Data)week4_cht_milf_normal,
		(IO_Data)week4_cht_milf_hard,
	},
	{
		(IO_Data)week4_cht_test,
		(IO_Data)week4_cht_test,
		(IO_Data)week4_cht_test,
	},
};

//Characters
//Boyfriend
#include "character/bf.c"

//Mommy Mearest
#include "character/mom.c"

//Girlfriend
#include "character/gf.c"

static fixed_t Char_GF_GetParallax(Char_GF *this)
{
	(void)this;
	return FIXED_UNIT;
}

//Week 4 textures
static Gfx_Tex week4_tex_back0; //Front limo
static Gfx_Tex week4_tex_back1; //Back limo
static Gfx_Tex week4_tex_back2; //Car
static Gfx_Tex week4_tex_back3; //Sky left
static Gfx_Tex week4_tex_back4; //Sky right

//Car state
#define CAR_START_X FIXED_DEC(-500,1)
#define CAR_END_X    FIXED_DEC(500,1)
#define CAR_TIME_A FIXED_DEC(5,1)
#define CAR_TIME_B FIXED_DEC(14,1)

static fixed_t week4_car_x;
static fixed_t week4_car_timer;

//Henchmen state
static IO_Data week4_arc_hench_ptr[2];
static Gfx_Tex week4_tex_hench;
static u8 week4_hench_frame, week4_hench_tex_id;

//Henchmen assets
static const CharFrame henchmen_frame[] = {
	{0, {  0,   0,  99,  99}, { 71,  98}}, //0 left 1
	{0, { 99,   0,  99,  98}, { 71,  97}}, //1 left 2
	{0, {  0,  99,  98,  97}, { 69,  95}}, //2 left 3
	{0, { 98,  98,  62,  89}, { 42,  88}}, //3 left 4
	{0, {160,  98,  62,  89}, { 42,  88}}, //4 left 5
	
	{1, {  0,   0, 101, 103}, { 35, 101}}, //5 right 1
	{1, {101,   0,  99, 101}, { 33, 100}}, //6 right 2
	{1, {  0, 103,  99, 101}, { 33,  99}}, //7 right 3
	{1, { 99, 101,  64,  90}, { 26,  89}}, //8 right 4
	{1, {163, 101,  64,  90}, { 26,  89}}, //9 right 5
};

static const Animation henchmen_anim[] = {
	{1, (const u8[]){0, 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 4, ASCR_BACK, 1}}, //Left
	{1, (const u8[]){5, 5, 6, 6, 6, 7, 7, 7, 7, 8, 8, 9, ASCR_BACK, 1}}, //Right
};

static u8 week4_arc_hench[] = {
	#include "iso/week4/hench.arc.h"
};

//Henchmen functions
void Week4_Henchmen_SetFrame(void *user, u8 frame)
{
	(void)user;
	
	//Check if this is a new frame
	if (frame != week4_hench_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &henchmen_frame[week4_hench_frame = frame];
		if (cframe->tex != week4_hench_tex_id)
			Gfx_LoadTex(&week4_tex_hench, week4_arc_hench_ptr[week4_hench_tex_id = cframe->tex], 0);
	}
}

void Week4_Henchmen_Draw(fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &henchmen_frame[week4_hench_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&week4_tex_hench, &src, &dst, stage.camera.bzoom);
}

static Animatable week4_hench_animatable;

//Week 4 background functions
static void Week4_Load(void)
{
	//Load assets
	IO_Data overlay_data;
	
	Gfx_LoadTex(&stage.tex_hud0, overlay_data = Overlay_DataRead(), 0); Mem_Free(overlay_data); //hud0.tim
	Gfx_LoadTex(&stage.tex_hud1, overlay_data = Overlay_DataRead(), 0); Mem_Free(overlay_data); //hud1.tim
	
	Gfx_LoadTex(&week4_tex_back0, overlay_data = Overlay_DataRead(), 0); Mem_Free(overlay_data); //back0.tim
	Gfx_LoadTex(&week4_tex_back1, overlay_data = Overlay_DataRead(), 0); Mem_Free(overlay_data); //back1.tim
	Gfx_LoadTex(&week4_tex_back2, overlay_data = Overlay_DataRead(), 0); Mem_Free(overlay_data); //back2.tim
	Gfx_LoadTex(&week4_tex_back3, overlay_data = Overlay_DataRead(), 0); Mem_Free(overlay_data); //back3.tim
	Gfx_LoadTex(&week4_tex_back4, overlay_data = Overlay_DataRead(), 0); Mem_Free(overlay_data); //back4.tim
	
	//Load characters
	stage.player = Char_BF_New(FIXED_DEC(120,1), FIXED_DEC(40,1));
	stage.opponent = Char_Mom_New(FIXED_DEC(-120,1), FIXED_DEC(100,1));
	stage.gf = Char_GF_New(FIXED_DEC(0,1), FIXED_DEC(-10,1));
	
	//Load henchmen textures
	week4_arc_hench_ptr[0] = Archive_Find((IO_Data)week4_arc_hench, "hench0.tim");
	week4_arc_hench_ptr[1] = Archive_Find((IO_Data)week4_arc_hench, "hench1.tim");
	
	//Initialize car state
	week4_car_x = CAR_END_X;
	week4_car_timer = RandomRange(CAR_TIME_A, CAR_TIME_B);
	
	//Initialize henchmen state
	Animatable_Init(&week4_hench_animatable, henchmen_anim);
	Animatable_SetAnim(&week4_hench_animatable, 0);
	week4_hench_frame = week4_hench_tex_id = 0xFF; //Force art load
}

static void Week4_DrawBG()
{
	fixed_t fx, fy;
	
	//Animate and draw henchmen
	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		switch (stage.song_step & 7)
		{
			case 0:
				Animatable_SetAnim(&week4_hench_animatable, 0);
				break;
			case 4:
				Animatable_SetAnim(&week4_hench_animatable, 1);
				break;
		}
	}
	Animatable_Animate(&week4_hench_animatable, NULL, Week4_Henchmen_SetFrame);
	
	Week4_Henchmen_Draw(FIXED_DEC(-50,1) - fx, FIXED_DEC(30,1) - fy);
	Week4_Henchmen_Draw( FIXED_DEC(50,1) - fx, FIXED_DEC(30,1) - fy);
	Week4_Henchmen_Draw(FIXED_DEC(150,1) - fx, FIXED_DEC(30,1) - fy);
	Week4_Henchmen_Draw(FIXED_DEC(250,1) - fx, FIXED_DEC(30,1) - fy);
	
	//Draw background limo
	//Use same scroll as henchmen
	RECT bglimo_src = {0, 0, 255, 95};
	RECT_FIXED bglimo_dst = {
		FIXED_DEC(-210,1) - fx,
		FIXED_DEC(30,1) - fy,
		FIXED_DEC(255,1),
		FIXED_DEC(95,1)
	};
	
	Stage_DrawTex(&week4_tex_back1, &bglimo_src, &bglimo_dst, stage.camera.bzoom);
	bglimo_dst.x += bglimo_dst.w;
	bglimo_src.y += 95;
	Stage_DrawTex(&week4_tex_back1, &bglimo_src, &bglimo_dst, stage.camera.bzoom);
	
	//Draw sunset
	fx = stage.camera.x >> 4;
	fy = stage.camera.y >> 4;
	
	RECT sunset_src = {0, 0, 255, 255};
	RECT_FIXED sunset_dst = {
		FIXED_DEC(-250,1) - fx,
		FIXED_DEC(-130,1) - fy,
		FIXED_DEC(255,1),
		FIXED_DEC(255,1)
	};
	
	Stage_DrawTex(&week4_tex_back3, &sunset_src, &sunset_dst, stage.camera.bzoom);
	sunset_dst.x += sunset_dst.w;
	Stage_DrawTex(&week4_tex_back4, &sunset_src, &sunset_dst, stage.camera.bzoom);
}

void Week4_DrawMD()
{
	fixed_t fx, fy;
	
	//Draw foreground limo
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT fglimo_src = {0, 0, 255, 127};
	RECT_FIXED fglimo_dst = {
		FIXED_DEC(-220,1) - fx,
		FIXED_DEC(50,1) - fy,
		FIXED_DEC(256,1),
		FIXED_DEC(128,1)
	};
	
	Stage_DrawTex(&week4_tex_back0, &fglimo_src, &fglimo_dst, stage.camera.bzoom);
	fglimo_dst.x += fglimo_dst.w;
	fglimo_dst.y -= (fglimo_dst.h * 22) >> 7;
	fglimo_src.y += 128;
	Stage_DrawTex(&week4_tex_back0, &fglimo_src, &fglimo_dst, stage.camera.bzoom);
}

static void Week4_DrawFG()
{
	fixed_t fx, fy;
	
	//Move car
	week4_car_timer -= timer_dt;
	if (week4_car_timer <= 0)
	{
		week4_car_timer = RandomRange(CAR_TIME_A, CAR_TIME_B);
		week4_car_x = CAR_START_X;
	}
	
	if (week4_car_x < CAR_END_X)
		week4_car_x += timer_dt * 2700;
	
	//Draw car
	fx = stage.camera.x * 4 / 3;
	fy = stage.camera.y * 4 / 3;
	
	RECT car_src = {0, 0, 255, 89};
	RECT_FIXED car_dst = {
		week4_car_x - fx,
		FIXED_DEC(60,1) - fy,
		FIXED_DEC(400,1),
		FIXED_DEC(140,1)
	};
	
	Stage_DrawTex(&week4_tex_back2, &car_src, &car_dst, stage.camera.bzoom);
}

static IO_Data Week4_GetChart(void)
{
	return week4_cht[stage.stage_id - StageId_4_1][stage.stage_diff];
}

static boolean Week4_LoadScreen(void)
{
	return false;
}

static boolean Week4_NextStage(void)
{
	switch (stage.stage_id)
	{
		case StageId_4_1: //Satin Panties
			stage.stage_id = StageId_4_2;
			return true;
		case StageId_4_2: //High
			stage.stage_id = StageId_4_3;
			return true;
		case StageId_4_3: //M.I.L.F
			return false;
		default:
			return false;
	}
}

void Week4_SetPtr(void)
{
	//Set pointers
	stageoverlay_load = Week4_Load;
	stageoverlay_tick = NULL;
	stageoverlay_drawbg = Week4_DrawBG;
	stageoverlay_drawmd = Week4_DrawMD;
	stageoverlay_drawfg = Week4_DrawFG;
	stageoverlay_free = NULL;
	stageoverlay_getchart = Week4_GetChart;
	stageoverlay_loadscreen = Week4_LoadScreen;
	stageoverlay_nextstage = Week4_NextStage;
}
