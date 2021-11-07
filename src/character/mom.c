/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "boot/character.h"
#include "boot/mem.h"
#include "boot/archive.h"
#include "boot/stage.h"
#include "boot/main.h"
#include "boot/timer.h"

//Mom character assets
static u8 char_mom_arc_main[] = {
	#include "iso/mom/main.arc.h"
};
static u8 char_mom_tim_hair[] = {
	#include "iso/mom/hair.tim.h"
};

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
	{Mom_ArcMain_Idle0, {  0,   0,  83, 177}, { 42, 163}}, //0 idle 1
	{Mom_ArcMain_Idle0, { 84,   0,  82, 179}, { 41, 164}}, //1 idle 2
	{Mom_ArcMain_Idle1, {  0,   0,  82, 180}, { 41, 165}}, //2 idle 3
	{Mom_ArcMain_Idle1, { 83,   0,  82, 180}, { 41, 165}}, //3 idle 4
	
	{Mom_ArcMain_Left, {  0,   0, 127, 166}, { 70, 151}}, //4 left 1
	{Mom_ArcMain_Left, {128,   0, 124, 167}, { 68, 152}}, //5 left 1
	
	{Mom_ArcMain_Down, {  0,   0,  97, 113}, { 41, 111}}, //6 down 1
	{Mom_ArcMain_Down, { 98,   0,  91, 116}, { 38, 114}}, //7 down 2
	
	{Mom_ArcMain_Up, {  0,   0,  71, 200}, { 34, 196}}, //8 up 1
	{Mom_ArcMain_Up, { 72,   0,  71, 198}, { 35, 193}}, //9 up 2
	
	{Mom_ArcMain_Right, {  0,   0, 122, 163}, { 62, 150}}, //10 right 1
	{Mom_ArcMain_Right, {123,   0, 117, 164}, { 61, 151}}, //11 right 2
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
static void Char_Mom_SetFrame(void *user, u8 frame)
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

static void Char_Mom_Tick(Character *character)
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
		
		{2,  87, 197}, //left 1
		{2,  86, 197}, //left 2
		
		{2,  48, 159}, //down 1
		{2,  48, 161}, //down 2
		
		{0,  60, 215}, //up 1
		{0,  59, 214}, //up 2
		
		{0,  16, 182}, //right 1
		{0,  15, 182}, //right 2
	};
	
	static const RECT hair_srcs[] = {
		{  0,   0, 115,  99},
		{116,   0, 113,  94},
		{  0, 100, 117, 103},
		{118,  95, 115,  98},
	};
	
	static const u8 hair_offs[][2] = {
		{ 5,  5},
		{ 7,  9},
		{ 7,  8},
		{ 7, 10}
	};
	
	const struct Char_Mom_HairDef *hair_def = &hair_defs[this->frame];
	u8 hair_i = (animf_count & 1) | hair_def->sy;
	
	const RECT *hair_src = &hair_srcs[hair_i];
	RECT_FIXED hair_dst = {
		character->x - FIXED_DEC(hair_def->ox,1) + FIXED_DEC(hair_offs[hair_i][0],1) - stage.camera.x,
		character->y - FIXED_DEC(hair_def->oy,1) + FIXED_DEC(hair_offs[hair_i][1],1) - stage.camera.y,
		FIXED_DEC(hair_src->w,1),
		FIXED_DEC(hair_src->h,1)
	};
	
	Stage_DrawTex(&this->tex_hair, hair_src, &hair_dst, stage.camera.bzoom);
}

static void Char_Mom_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

static void Char_Mom_Free(Character *character)
{
	(void)character;
}

static Character *Char_Mom_New(fixed_t x, fixed_t y)
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
	Gfx_LoadTex(&this->tex_hair, (IO_Data)char_mom_tim_hair, 0);
	
	//Load art
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
		*arc_ptr++ = Archive_Find((IO_Data)char_mom_arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
