/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "gf.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

#include "speaker.h"

#include "../stage/week7.h"

//GF Weeb character structure
enum
{
	GFWeeb_ArcMain_Weeb0,
	GFWeeb_ArcMain_Weeb1,
	
	GFWeeb_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[GFWeeb_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_GFWeeb;

//GF character definitions
static const CharFrame char_gfweeb_frame[] = {
	{GFWeeb_ArcMain_Weeb0, {  0,   0, 104,  92}, { 52,  92}}, //0 left 0
	{GFWeeb_ArcMain_Weeb0, {105,   0, 103,  93}, { 52,  93}}, //1 left 1
	{GFWeeb_ArcMain_Weeb0, {  0,  93,  98,  96}, { 49,  96}}, //2 left 2
	{GFWeeb_ArcMain_Weeb0, { 99,  94,  98,  96}, { 49,  96}}, //3 left 3
	
	{GFWeeb_ArcMain_Weeb1, {  0,   0, 104,  91}, { 52,  91}}, //4 right 0
	{GFWeeb_ArcMain_Weeb1, {105,   0, 102,  92}, { 51,  92}}, //5 right 1
	{GFWeeb_ArcMain_Weeb1, {  0,  92,  98,  96}, { 49,  96}}, //6 right 2
	{GFWeeb_ArcMain_Weeb1, { 99,  93,  98,  95}, { 49,  95}}, //7 right 3
};

static const Animation char_gfweeb_anim[CharAnim_Max] = {
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Idle
	{1, (const u8[]){ 0,  0,  1,  1,  1,  2,  2,  2,  2,  3, ASCR_BACK, 1}}, //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_LeftAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_UpAlt
	{1, (const u8[]){ 4,  4,  5,  5,  5,  6,  6,  6,  6,  7, ASCR_BACK, 1}}, //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                           //CharAnim_RightAlt
};

//GF Weeb character functions
void Char_GFWeeb_SetFrame(void *user, u8 frame)
{
	Char_GFWeeb *this = (Char_GFWeeb*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_gfweeb_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_GFWeeb_Tick(Character *character)
{
	Char_GFWeeb *this = (Char_GFWeeb*)character;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform dance
		if ((stage.song_step % stage.gf_speed) == 0)
		{
			//Switch animation
			if (character->animatable.anim == CharAnim_Left)
				character->set_anim(character, CharAnim_Right);
			else
				character->set_anim(character, CharAnim_Left);
		}
	}
	
	//Get parallax
	fixed_t parallax;
	if (stage.stage_id == StageId_6_3)
		parallax = FIXED_DEC(7,10);
	else
		parallax = FIXED_DEC(85,100);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_GFWeeb_SetFrame);
	Character_DrawParallax(character, &this->tex, &char_gfweeb_frame[this->frame], parallax);
}

void Char_GFWeeb_SetAnim(Character *character, u8 anim)
{
	//Set animation
	if (anim != CharAnim_Idle && anim != CharAnim_Left && anim != CharAnim_Right)
		return;
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_GFWeeb_Free(Character *character)
{
	Char_GFWeeb *this = (Char_GFWeeb*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_GFWeeb_New(fixed_t x, fixed_t y)
{
	//Allocate gf weeb object
	Char_GFWeeb *this = Mem_Alloc(sizeof(Char_GFWeeb));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_GFWeeb_New] Failed to allocate gf weeb object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_GFWeeb_Tick;
	this->character.set_anim = Char_GFWeeb_SetAnim;
	this->character.free = Char_GFWeeb_Free;
	
	Animatable_Init(&this->character.animatable, char_gfweeb_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(16,1);
	this->character.focus_y = FIXED_DEC(-50,1);
	this->character.focus_zoom = FIXED_DEC(13,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\GFWEEB.ARC;1");
	
	const char **pathp = (const char *[]){
		"weeb0.tim",  //GFWeeb_ArcMain_Weeb0
		"weeb1.tim",  //GFWeeb_ArcMain_Weeb1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
