/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mom.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"
#include "../timer.h"

//Mom character structure
enum
{
	Mom_ArcMain_Idle0,
	Mom_ArcMain_Idle1,
	Mom_ArcMain_Left,
	Mom_ArcMain_Down,
	Mom_ArcMain_Up,
	Mom_ArcMain_Right,
	
	Mom_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Mom_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
	//Hair texture
	Gfx_Tex tex_hair;
} Char_Mom;

//Mom character definitions
static const CharFrame char_mom_frame[] = {
	{Mom_ArcMain_Idle0, {  0,   0, 128, 256}, { 42, 163}}, //0 idle 1
	{Mom_ArcMain_Idle0, {128,   0, 128, 256}, { 41, 164}}, //1 idle 2
	{Mom_ArcMain_Idle1, {  0,   0, 128, 256}, { 41, 165}}, //2 idle 3
	{Mom_ArcMain_Idle1, {128,   0, 128, 256}, { 41, 165}}, //3 idle 4
	
	{Mom_ArcMain_Left, {  0,   0, 128, 256}, { 65, 151}}, //4 left 1
	{Mom_ArcMain_Left, {128,   0, 128, 256}, { 63, 152}}, //5 left 1
	
	{Mom_ArcMain_Down, {  0,   0, 128, 128}, { 41, 111}}, //6 down 1
	{Mom_ArcMain_Down, {128,   0, 128, 128}, { 42, 114}}, //7 down 2
	
	{Mom_ArcMain_Up, {  0,   0, 128, 256}, { 34, 196}}, //8 up 1
	{Mom_ArcMain_Up, {128,   0, 128, 256}, { 35, 193}}, //9 up 2
	
	{Mom_ArcMain_Right, {  0,   0, 128, 256}, { 62, 150}}, //10 right 1
	{Mom_ArcMain_Right, {128,   0, 128, 256}, { 61, 151}}, //11 right 2
};

static const Animation char_mom_anim[CharAnim_Max] = {
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

//Mom character functions
void Char_Mom_SetFrame(void *user, u8 frame)
{
	Char_Mom *this = (Char_Mom*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_mom_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Mom_Tick(Character *character)
{
	Char_Mom *this = (Char_Mom*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Mom_SetFrame);
	Character_Draw(character, &this->tex, &char_mom_frame[this->frame]);
	
	//Draw hair
	static const struct Char_Mom_HairDef
	{
		boolean sy;
		u8 ox, oy;
	} hair_defs[] = {
		{0,  43, 196}, //idle 1
		{0,  43, 196}, //idle 2
		{0,  43, 197}, //idle 3
		{0,  43, 197}, //idle 4
		
		{1,  87, 197}, //left 1
		{1,  86, 197}, //left 2
		
		{1,  43, 159}, //down 1
		{1,  43, 161}, //down 2
		
		{0,  60, 215}, //up 1
		{0,  59, 214}, //up 2
		
		{0,  16, 182}, //right 1
		{0,  15, 182}, //right 2
	};
	
	const struct Char_Mom_HairDef *hair_def = &hair_defs[this->frame];
	RECT hair_src = {
		(animf_count & 1) << 7,
		hair_def->sy << 7,
		128,
		128
	};
	RECT_FIXED hair_dst = {
		character->x - ((fixed_t)hair_def->ox << FIXED_SHIFT) - stage.camera.x,
		character->y - ((fixed_t)hair_def->oy << FIXED_SHIFT) - stage.camera.y,
		FIXED_DEC(128,1),
		FIXED_DEC(128,1)
	};
	
	Stage_DrawTex(&this->tex_hair, &hair_src, &hair_dst, stage.camera.bzoom);
}

void Char_Mom_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Mom_Free(Character *character)
{
	Char_Mom *this = (Char_Mom*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Mom_New(fixed_t x, fixed_t y)
{
	//Allocate mom object
	Char_Mom *this = Mem_Alloc(sizeof(Char_Mom));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Mom_New] Failed to allocate mom object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Mom_Tick;
	this->character.set_anim = Char_Mom_SetAnim;
	this->character.free = Char_Mom_Free;
	
	Animatable_Init(&this->character.animatable, char_mom_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 4;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load hair art
	Gfx_LoadTex(&this->tex_hair, IO_Read("\\CHAR\\MOMHAIR.TIM;1"), GFX_LOADTEX_FREE);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MOM.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Mom_ArcMain_Idle0
		"idle1.tim", //Mom_ArcMain_Idle1
		"left.tim",  //Mom_ArcMain_Left
		"down.tim",  //Mom_ArcMain_Down
		"up.tim",    //Mom_ArcMain_Up
		"right.tim", //Mom_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
