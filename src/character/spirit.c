/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "spirit.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"
#include "../random.h"
#include "../mutil.h"

//Dad character structure
enum
{
	Spirit_ArcMain_Spirit0,
	Spirit_ArcMain_Spirit1,
	
	Spirit_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Spirit_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
	//Distort state
	fixed_t distort_ang, distort_pow, distort_spd;
	fixed_t ghost_x, ghost_y;
} Char_Spirit;

//Dad character definitions
static const CharFrame char_spirit_frame[] = {
	{Spirit_ArcMain_Spirit0, {  0,   0,  48, 104}, { 24, 50 + 52}}, //0 idle 1
	{Spirit_ArcMain_Spirit0, { 49,   0,  49, 108}, { 25, 50 + 54}}, //1 idle 2
	{Spirit_ArcMain_Spirit0, { 99,   0,  49, 106}, { 24, 50 + 53}}, //2 idle 3
	{Spirit_ArcMain_Spirit0, {149,   0,  48, 104}, { 25, 50 + 52}}, //3 idle 4
	
	{Spirit_ArcMain_Spirit0, {198,   0,  41,  99}, { 31, 50 + 50}}, //4 left 1
	{Spirit_ArcMain_Spirit0, {  0, 109,  51, 100}, { 25, 50 + 51}}, //5 left 2
	{Spirit_ArcMain_Spirit0, { 52, 109,  51, 100}, { 25, 50 + 51}}, //6 left 3
	
	{Spirit_ArcMain_Spirit0, {104, 107,  48, 131}, { 22, 50 + 55}}, //7 down 1
	{Spirit_ArcMain_Spirit0, {153, 105,  46, 113}, { 21, 50 + 57}}, //8 down 2
	{Spirit_ArcMain_Spirit0, {200, 100,  47, 110}, { 22, 50 + 58}}, //9 down 3
	
	{Spirit_ArcMain_Spirit1, {  0,   0,  43, 110}, { 22, 50 + 65}}, //10 up 1
	{Spirit_ArcMain_Spirit1, { 44,   0,  42, 125}, { 21, 50 + 64}}, //11 up 2
	{Spirit_ArcMain_Spirit1, { 87,   0,  43, 126}, { 22, 50 + 63}}, //12 up 3
	
	{Spirit_ArcMain_Spirit1, {131,   0,  60, 109}, { 15, 50 + 56}}, //13 right 1
	{Spirit_ArcMain_Spirit1, {192,   0,  50, 105}, { 20, 50 + 53}}, //14 right 2
	{Spirit_ArcMain_Spirit1, {  0, 111,  54, 111}, { 19, 50 + 55}}, //15 right 3
};

static const Animation char_spirit_anim[CharAnim_Max] = {
	{1, (const u8[]){ 0,  0,  1,  1,  2,  2,  3,  3, ASCR_REPEAT}}, //CharAnim_Idle
	{1, (const u8[]){ 4,  4,  5,  5,  6,  6, ASCR_BACK, 4}},        //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                  //CharAnim_LeftAlt
	{1, (const u8[]){ 7,  8,  8,  9,  9, ASCR_BACK, 4}},            //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                  //CharAnim_DownAlt
	{1, (const u8[]){10, 11, 11, 12, 12, ASCR_BACK, 4}},            //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                  //CharAnim_UpAlt
	{1, (const u8[]){13, 14, 14, 15, 15, ASCR_BACK, 4}},            //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                  //CharAnim_RightAlt
};

//Dad character functions
void Char_Spirit_SetFrame(void *user, u8 frame)
{
	Char_Spirit *this = (Char_Spirit*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_spirit_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
	
	//Process distortion
	this->distort_ang += this->distort_spd;
	this->distort_pow += (FIXED_UNIT - this->distort_pow) >> 2;
	this->distort_spd += (FIXED_UNIT - this->distort_spd) >> 1;
	
	this->ghost_x += (FIXED_UNIT - this->ghost_x) >> 2;
	this->ghost_y += (FIXED_UNIT - this->ghost_y) >> 2;
}

static void Char_Spirit_Draw(Char_Spirit *this, fixed_t x, fixed_t y, fixed_t phase, boolean mode)
{
	//Get character state stuff
	const CharFrame *cframe = &char_spirit_frame[this->frame];
	
	//Get offset coordinates
	fixed_t ox = x - stage.camera.x - FIXED_DEC(cframe->off[0],1);
	fixed_t oy = y - stage.camera.y - FIXED_DEC(cframe->off[1],1);
	
	//Get distorted points
	fixed_t pang = FIXED_MUL(this->distort_ang, phase) >> FIXED_SHIFT;
	u8 a0 = pang *  9 / 10;
	u8 a1 = pang * 11 / 10;
	u8 a2 = pang * 10 / 10;
	u8 a3 = pang * 12 / 10;
	
	fixed_t pow = this->distort_pow * 2;
	
	POINT_FIXED d0 = {ox + ((MUtil_Cos(a0) * pow) >> 8),                               oy + ((MUtil_Sin(a0) * pow) >> 8)};
	POINT_FIXED d1 = {ox + ((MUtil_Cos(a1) * pow) >> 8) + FIXED_DEC(cframe->src[2],1), oy + ((MUtil_Sin(a1) * pow) >> 8)};
	POINT_FIXED d2 = {ox + ((MUtil_Cos(a2) * pow) >> 8),                               oy + ((MUtil_Sin(a2) * pow) >> 8) + FIXED_DEC(cframe->src[3],1)};
	POINT_FIXED d3 = {ox + ((MUtil_Cos(a3) * pow) >> 8) + FIXED_DEC(cframe->src[2],1), oy + ((MUtil_Sin(a3) * pow) >> 8) + FIXED_DEC(cframe->src[3],1)};
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	if (mode)
		Stage_BlendTexArb(&this->tex, &src, &d0, &d1, &d2, &d3, stage.camera.bzoom, 0);
	else
		Stage_DrawTexArb(&this->tex, &src, &d0, &d1, &d2, &d3, stage.camera.bzoom);
}

void Char_Spirit_Tick(Character *character)
{
	Char_Spirit *this = (Char_Spirit*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate
	Animatable_Animate(&character->animatable, (void*)this, Char_Spirit_SetFrame);
	
	//Draw body and ghost
	Char_Spirit_Draw(this, character->x, character->y, FIXED_DEC(25,10), false);
	Char_Spirit_Draw(this, character->x + this->ghost_x, character->y + this->ghost_y, FIXED_DEC(15,10), true);
}

void Char_Spirit_SetAnim(Character *character, u8 anim)
{
	Char_Spirit *this = (Char_Spirit*)character;
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
	
	//Increase distortion
	this->distort_pow += FIXED_DEC(14,10);
	this->distort_spd += FIXED_DEC(145,10);
	
	switch (anim)
	{
		case CharAnim_Idle:
			this->ghost_x += RandomRange(FIXED_DEC(-9,1), FIXED_DEC(9,1));
			this->ghost_y += RandomRange(FIXED_DEC(-13,1), FIXED_DEC(13,1));
			break;
		case CharAnim_Left:
			this->ghost_x += RandomRange(FIXED_DEC(2,1), FIXED_DEC(16,1));
			break;
		case CharAnim_Down:
			this->ghost_y -= RandomRange(FIXED_DEC(2,1), FIXED_DEC(16,1));
			break;
		case CharAnim_Up:
			this->ghost_y += RandomRange(FIXED_DEC(2,1), FIXED_DEC(16,1));
			break;
		case CharAnim_Right:
			this->ghost_x -= RandomRange(FIXED_DEC(2,1), FIXED_DEC(16,1));
			break;
	}
}

void Char_Spirit_Free(Character *character)
{
	Char_Spirit *this = (Char_Spirit*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Spirit_New(fixed_t x, fixed_t y)
{
	//Allocate spirit object
	Char_Spirit *this = Mem_Alloc(sizeof(Char_Spirit));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Spirit_New] Failed to allocate spirit object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Spirit_Tick;
	this->character.set_anim = Char_Spirit_SetAnim;
	this->character.free = Char_Spirit_Free;
	
	Animatable_Init(&this->character.animatable, char_spirit_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 9;
	
	this->character.focus_x = FIXED_DEC(24,1);
	this->character.focus_y = FIXED_DEC(-55,1);
	this->character.focus_zoom = FIXED_DEC(2,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SPIRIT.ARC;1");
	
	const char **pathp = (const char *[]){
		"spirit0.tim", //Spirit_ArcMain_Spirit0
		"spirit1.tim", //Spirit_ArcMain_Spirit1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize distort speed
	this->distort_ang = 0;
	this->distort_pow = FIXED_UNIT;
	this->distort_spd = FIXED_UNIT;
	
	this->ghost_x = this->ghost_y = 0;
	
	return (Character*)this;
}
