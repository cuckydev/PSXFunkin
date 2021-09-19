/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "senpaim.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Dad character structure
enum
{
	SenpaiM_ArcMain_SenpaiM0,
	SenpaiM_ArcMain_SenpaiM1,
	
	SenpaiM_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[SenpaiM_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_SenpaiM;

//Dad character definitions
static const CharFrame char_senpaim_frame[] = {
	{SenpaiM_ArcMain_SenpaiM0, {  0,   0,  66, 112}, { 32, 107}}, //0 idle 1
	{SenpaiM_ArcMain_SenpaiM0, { 67,   0,  65, 112}, { 31, 107}}, //1 idle 2
	{SenpaiM_ArcMain_SenpaiM0, {133,   0,  65, 113}, { 31, 108}}, //2 idle 3
	{SenpaiM_ArcMain_SenpaiM0, {  0, 113,  67, 116}, { 31, 111}}, //3 idle 4
	{SenpaiM_ArcMain_SenpaiM0, { 71, 113,  67, 115}, { 31, 110}}, //4 idle 5
	
	{SenpaiM_ArcMain_SenpaiM1, {  0,   0,  59, 115}, { 33, 110}}, //5 left 1
	{SenpaiM_ArcMain_SenpaiM1, { 60,   0,  61, 115}, { 35, 110}}, //6 left 2
	
	{SenpaiM_ArcMain_SenpaiM1, {122,   0,  63, 107}, { 31, 102}}, //7 down 1
	{SenpaiM_ArcMain_SenpaiM1, {186,   1,  62, 108}, { 30, 103}}, //8 down 2
	
	{SenpaiM_ArcMain_SenpaiM1, {  0, 116,  64, 122}, { 31, 115}}, //9 up 1
	{SenpaiM_ArcMain_SenpaiM1, { 65, 116,  66, 122}, { 32, 115}}, //10 up 2
	
	{SenpaiM_ArcMain_SenpaiM1, {132, 110,  65, 114}, { 27, 109}}, //11 right 1
	{SenpaiM_ArcMain_SenpaiM0, {139, 114,  64, 114}, { 28, 109}}, //12 right 2
};

static const Animation char_senpaim_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5,  6, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){ 9, 10, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){11, 12, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
};

//Dad character functions
void Char_SenpaiM_SetFrame(void *user, u8 frame)
{
	Char_SenpaiM *this = (Char_SenpaiM*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_senpaim_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_SenpaiM_Tick(Character *character)
{
	Char_SenpaiM *this = (Char_SenpaiM*)character;
	
	//Camera stuff
	if (stage.story)
	{
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
			if (stage.song_step < 4)
			{
				this->character.focus_x = FIXED_DEC(50,1);
				this->character.focus_y = FIXED_DEC(-55,1);
			}
			else
			{
				this->character.focus_x = FIXED_DEC(24,1);
				this->character.focus_y = FIXED_DEC(-66,1);
			}
		}
	}
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_SenpaiM_SetFrame);
	Character_Draw(character, &this->tex, &char_senpaim_frame[this->frame]);
}

void Char_SenpaiM_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_SenpaiM_Free(Character *character)
{
	Char_SenpaiM *this = (Char_SenpaiM*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_SenpaiM_New(fixed_t x, fixed_t y)
{
	//Allocate senpaim object
	Char_SenpaiM *this = Mem_Alloc(sizeof(Char_SenpaiM));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_SenpaiM_New] Failed to allocate senpaim object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_SenpaiM_Tick;
	this->character.set_anim = Char_SenpaiM_SetAnim;
	this->character.free = Char_SenpaiM_Free;
	
	Animatable_Init(&this->character.animatable, char_senpaim_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 8;
	
	this->character.focus_x = FIXED_DEC(24,1);
	this->character.focus_y = FIXED_DEC(-66,1);
	this->character.focus_zoom = FIXED_DEC(2,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SENPAIM.ARC;1");
	
	const char **pathp = (const char *[]){
		"senpai0.tim", //SenpaiM_ArcMain_SenpaiM0
		"senpai1.tim", //SenpaiM_ArcMain_SenpaiM1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
