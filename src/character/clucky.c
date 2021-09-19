/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "clucky.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Clucky character structure
enum
{
	Clucky_ArcMain_Idle0,
	Clucky_ArcMain_Idle1,
	Clucky_ArcMain_Left,
	Clucky_ArcMain_Down,
	Clucky_ArcMain_Up,
	Clucky_ArcMain_Right,
	
	Clucky_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Clucky_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Clucky;

//Clucky character definitions
static const CharFrame char_clucky_frame[] = {
	{Clucky_ArcMain_Idle0, {  0,   0, 109, 165}, { 42, 163}}, //0 idle 1
	{Clucky_ArcMain_Idle0, {110,   0, 106, 166}, { 41, 164}}, //1 idle 2
	{Clucky_ArcMain_Idle1, {  0,   0, 106, 169}, { 41, 167}}, //2 idle 3
	{Clucky_ArcMain_Idle1, {107,   0, 106, 167}, { 41, 165}}, //3 idle 4
	
	{Clucky_ArcMain_Left, {  0,   0, 103, 166}, { 50, 165}}, //4 left 1
	{Clucky_ArcMain_Left, {104,   0, 104, 166}, { 48, 165}}, //5 left 2
	
	{Clucky_ArcMain_Down, {  0,   0, 109, 159}, { 47, 158}}, //6 down 1
	{Clucky_ArcMain_Down, {110,   0, 108, 162}, { 44, 161}}, //7 down 2
	
	{Clucky_ArcMain_Up, {  0,   0,  95, 176}, { 32, 175}}, //8 up 1
	{Clucky_ArcMain_Up, { 96,   0,  97, 173}, { 32, 172}}, //9 up 2
	
	{Clucky_ArcMain_Right, {  0,   0, 126, 166}, { 53, 165}}, //10 right 1
	{Clucky_ArcMain_Right, {127,   0, 120, 167}, { 51, 166}}, //11 right 2
};

static const Animation char_clucky_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Clucky character functions
void Char_Clucky_SetFrame(void *user, u8 frame)
{
	Char_Clucky *this = (Char_Clucky*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_clucky_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Clucky_Tick(Character *character)
{
	Char_Clucky *this = (Char_Clucky*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Clucky_SetFrame);
	Character_Draw(character, &this->tex, &char_clucky_frame[this->frame]);
}

void Char_Clucky_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Clucky_Free(Character *character)
{
	Char_Clucky *this = (Char_Clucky*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Clucky_New(fixed_t x, fixed_t y)
{
	//Allocate clucky object
	Char_Clucky *this = Mem_Alloc(sizeof(Char_Clucky));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Clucky_New] Failed to allocate clucky object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Clucky_Tick;
	this->character.set_anim = Char_Clucky_SetAnim;
	this->character.free = Char_Clucky_Free;
	
	Animatable_Init(&this->character.animatable, char_clucky_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 11;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-85,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\CLUCKY.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Clucky_ArcMain_Idle0
		"idle1.tim", //Clucky_ArcMain_Idle1
		"left.tim",  //Clucky_ArcMain_Left
		"down.tim",  //Clucky_ArcMain_Down
		"up.tim",    //Clucky_ArcMain_Up
		"right.tim", //Clucky_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
