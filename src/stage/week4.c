/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week4.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../timer.h"
#include "../animation.h"

//Week 4 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	IO_Data arc_hench, arc_hench_ptr[2];
	
	Gfx_Tex tex_back0; //Foreground limo
	Gfx_Tex tex_back1; //Background limo
	Gfx_Tex tex_back2; //Sunset
	Gfx_Tex tex_back3; //Car
	
	//Car state
	fixed_t car_x;
	fixed_t car_timer;
	
	//Henchmen state
	Gfx_Tex tex_hench;
	u8 hench_frame, hench_tex_id;
	
	Animatable hench_animatable;
} Back_Week4;

//Henchmen animation and rects
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

//Henchmen functions
void Week4_Henchmen_SetFrame(void *user, u8 frame)
{
	Back_Week4 *this = (Back_Week4*)user;
	
	//Check if this is a new frame
	if (frame != this->hench_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &henchmen_frame[this->hench_frame = frame];
		if (cframe->tex != this->hench_tex_id)
			Gfx_LoadTex(&this->tex_hench, this->arc_hench_ptr[this->hench_tex_id = cframe->tex], 0);
	}
}

void Week4_Henchmen_Draw(Back_Week4 *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &henchmen_frame[this->hench_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_hench, &src, &dst, stage.camera.bzoom);
}

//Week 4 background functions
#define CAR_START_X FIXED_DEC(-500,1)
#define CAR_END_X    FIXED_DEC(500,1)
#define CAR_TIME_A FIXED_DEC(5,1)
#define CAR_TIME_B FIXED_DEC(14,1)

void Back_Week4_DrawFG(StageBack *back)
{
	Back_Week4 *this = (Back_Week4*)back;
	
	fixed_t fx, fy;
	
	//Move car
	this->car_timer -= timer_dt;
	if (this->car_timer <= 0)
	{
		this->car_timer = RandomRange(CAR_TIME_A, CAR_TIME_B);
		this->car_x = CAR_START_X;
	}
	
	if (this->car_x < CAR_END_X)
		this->car_x += timer_dt * 2700;
	
	//Draw car
	fx = stage.camera.x * 4 / 3;
	fy = stage.camera.y * 4 / 3;
	
	RECT car_src = {0, 0, 256, 128};
	RECT_FIXED car_dst = {
		this->car_x - fx,
		FIXED_DEC(60,1) - fy,
		FIXED_DEC(400,1),
		FIXED_DEC(200,1)
	};
	
	Stage_DrawTex(&this->tex_back3, &car_src, &car_dst, stage.camera.bzoom);
}

void Back_Week4_DrawMD(StageBack *back)
{
	Back_Week4 *this = (Back_Week4*)back;
	
	fixed_t fx, fy;
	
	//Draw foreground limo
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT fglimo_src = {0, 0, 255, 128};
	RECT_FIXED fglimo_dst = {
		FIXED_DEC(-220,1) - fx,
		FIXED_DEC(50,1) - fy,
		FIXED_DEC(256,1),
		FIXED_DEC(128,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &fglimo_src, &fglimo_dst, stage.camera.bzoom);
	fglimo_dst.x += fglimo_dst.w;
	fglimo_dst.y -= (fglimo_dst.h * 22) >> 7;
	fglimo_src.y += 128;
	Stage_DrawTex(&this->tex_back0, &fglimo_src, &fglimo_dst, stage.camera.bzoom);
}

void Back_Week4_DrawBG(StageBack *back)
{
	Back_Week4 *this = (Back_Week4*)back;
	
	fixed_t fx, fy;
	
	//Animate and draw henchmen
	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		switch (stage.song_step & 7)
		{
			case 0:
				Animatable_SetAnim(&this->hench_animatable, 0);
				break;
			case 4:
				Animatable_SetAnim(&this->hench_animatable, 1);
				break;
		}
	}
	Animatable_Animate(&this->hench_animatable, (void*)this, Week4_Henchmen_SetFrame);
	
	Week4_Henchmen_Draw(this, FIXED_DEC(-50,1) - fx, FIXED_DEC(30,1) - fy);
	Week4_Henchmen_Draw(this,  FIXED_DEC(50,1) - fx, FIXED_DEC(30,1) - fy);
	Week4_Henchmen_Draw(this, FIXED_DEC(150,1) - fx, FIXED_DEC(30,1) - fy);
	Week4_Henchmen_Draw(this, FIXED_DEC(250,1) - fx, FIXED_DEC(30,1) - fy);
	
	//Draw background limo
	//Use same scroll as henchmen
	RECT bglimo_src = {0, 0, 255, 128};
	RECT_FIXED bglimo_dst = {
		FIXED_DEC(-210,1) - fx,
		FIXED_DEC(30,1) - fy,
		FIXED_DEC(256,1),
		FIXED_DEC(128,1)
	};
	
	Stage_DrawTex(&this->tex_back1, &bglimo_src, &bglimo_dst, stage.camera.bzoom);
	bglimo_dst.x += bglimo_dst.w;
	bglimo_src.y += 128;
	Stage_DrawTex(&this->tex_back1, &bglimo_src, &bglimo_dst, stage.camera.bzoom);
	
	//Draw sunset
	fx = stage.camera.x >> 4;
	fy = stage.camera.y >> 4;
	
	RECT sunset_src = {0, 0, 256, 256};
	RECT_FIXED sunset_dst = {
		FIXED_DEC(-165 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-140,1) - fy,
		FIXED_DEC(340 + SCREEN_WIDEOADD,1),
		FIXED_DEC(260,1)
	};
	
	Stage_DrawTex(&this->tex_back2, &sunset_src, &sunset_dst, stage.camera.bzoom);
}

void Back_Week4_Free(StageBack *back)
{
	Back_Week4 *this = (Back_Week4*)back;
	
	//Free henchmen archive
	Mem_Free(this->arc_hench);
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week4_New(void)
{
	//Allocate background structure
	Back_Week4 *this = (Back_Week4*)Mem_Alloc(sizeof(Back_Week4));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_Week4_DrawFG;
	this->back.draw_md = Back_Week4_DrawMD;
	this->back.draw_bg = Back_Week4_DrawBG;
	this->back.free = Back_Week4_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK4\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Mem_Free(arc_back);
	
	//Load henchmen textures
	this->arc_hench = IO_Read("\\WEEK4\\HENCH.ARC;1");
	this->arc_hench_ptr[0] = Archive_Find(this->arc_hench, "hench0.tim");
	this->arc_hench_ptr[1] = Archive_Find(this->arc_hench, "hench1.tim");
	
	//Initialize car state
	this->car_x = CAR_END_X;
	this->car_timer = RandomRange(CAR_TIME_A, CAR_TIME_B);
	
	//Initialize henchmen state
	Animatable_Init(&this->hench_animatable, henchmen_anim);
	Animatable_SetAnim(&this->hench_animatable, 0);
	this->hench_frame = this->hench_tex_id = 0xFF; //Force art load
	
	return (StageBack*)this;
}
