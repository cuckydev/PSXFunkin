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

//GF character structure
enum
{
	GF_ArcMain_GF0,
	GF_ArcMain_GF1,
	GF_ArcMain_GF2,
	
	GF_ArcScene_0, //tut0
	GF_ArcScene_1, //tut1
	
	GF_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_scene;
	IO_Data arc_ptr[GF_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
	//Speaker
	Speaker speaker;
	
	//Pico test
	u16 *pico_p;
} Char_GF;

//GF character definitions
static const CharFrame char_gf_frame[] = {
	{GF_ArcMain_GF0, {  0,   0,  74, 103}, { 37,  72}}, //0 bop left 1
	{GF_ArcMain_GF0, { 75,   0,  74, 103}, { 38,  72}}, //1 bop left 2
	{GF_ArcMain_GF0, {150,   0,  73, 102}, { 37,  72}}, //2 bop left 3
	{GF_ArcMain_GF0, {  0, 104,  73, 103}, { 36,  73}}, //3 bop left 4
	{GF_ArcMain_GF0, { 74, 104,  78, 105}, { 38,  75}}, //4 bop left 5
	{GF_ArcMain_GF0, {153, 103,  81, 106}, { 41,  76}}, //5 bop left 6
	
	{GF_ArcMain_GF1, {  0,   0,  81, 104}, { 40,  73}}, //6 bop right 1
	{GF_ArcMain_GF1, { 82,   0,  81, 104}, { 40,  73}}, //7 bop right 2
	{GF_ArcMain_GF1, {164,   0,  80, 103}, { 39,  73}}, //8 bop right 3
	{GF_ArcMain_GF1, {  0, 104,  79, 103}, { 38,  74}}, //9 bop right 4
	{GF_ArcMain_GF1, { 80, 105,  74, 104}, { 32,  74}}, //10 bop right 5
	{GF_ArcMain_GF1, {155, 104,  74, 104}, { 32,  74}}, //11 bop right 6
	
	{GF_ArcMain_GF2, {  0,   0,  73, 100}, { 34,  71}}, //12 cry 1
	{GF_ArcMain_GF2, { 74,   0,  73, 102}, { 35,  72}}, //13 cry 2
	{GF_ArcMain_GF2, {148,   0,  73, 102}, { 34,  72}}, //14 cry 3
	{GF_ArcMain_GF2, {  0, 101,  74, 102}, { 35,  72}}, //15 cry 4
	{GF_ArcMain_GF2, { 75, 102,  73, 102}, { 34,  72}}, //16 cry 5
	
	{GF_ArcScene_0, {  0,   0,  75, 102}, { 39,  71}}, //17 left 1
	{GF_ArcScene_0, { 76,   0,  77, 103}, { 41,  72}}, //18 left 2
	
	{GF_ArcScene_0, {154,   0,  79, 102}, { 37,  71}}, //19 down 1
	{GF_ArcScene_0, {  0, 103,  78, 104}, { 37,  72}}, //20 down 2
	
	{GF_ArcScene_0, { 79, 104,  79, 108}, { 39,  78}}, //21 up 1
	{GF_ArcScene_0, {159, 104,  79, 109}, { 39,  78}}, //22 up 2
	
	{GF_ArcScene_1, {  0,   0,  81, 102}, { 41,  71}}, //23 right 1
	{GF_ArcScene_1, { 81,   0,  76, 103}, { 36,  72}}, //24 right 2
	
	{GF_ArcScene_1, {158,   0,  75, 108}, { 36,  78}}, //25 cheer 1
	{GF_ArcScene_1, {  0, 103,  77, 107}, { 37,  77}}, //26 cheer 2
};

static const Animation char_gf_anim[CharAnim_Max] = {
	{0, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                        //CharAnim_Idle
	{2, (const u8[]){17, 18, ASCR_BACK, 1}},                                 //CharAnim_Left
	{1, (const u8[]){ 0,  0,  1,  1,  2,  2,  3,  4,  4,  5, ASCR_BACK, 1}}, //CharAnim_LeftAlt
	{2, (const u8[]){19, 20, ASCR_BACK, 1}},                                 //CharAnim_Down
	{1, (const u8[]){12, 13, 14, 15, 16, ASCR_REPEAT}},                      //CharAnim_DownAlt
	{2, (const u8[]){21, 22, ASCR_BACK, 1}},                                 //CharAnim_Up
	{2, (const u8[]){25, 26, ASCR_BACK, 1}},                                 //CharAnim_UpAlt
	{2, (const u8[]){23, 24, ASCR_BACK, 1}},                                 //CharAnim_Right
	{1, (const u8[]){ 6,  6,  7,  7,  8,  8,  9, 10, 10, 11, ASCR_BACK, 1}}, //CharAnim_RightAlt
};

//GF character functions
void Char_GF_SetFrame(void *user, u8 frame)
{
	Char_GF *this = (Char_GF*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_gf_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_GF_Tick(Character *character)
{
	Char_GF *this = (Char_GF*)character;
	
	//Initialize Pico test
	if (stage.stage_id == StageId_7_3 && stage.back != NULL && this->pico_p == NULL)
		this->pico_p = ((Back_Week7*)stage.back)->pico_chart;
	
	if (this->pico_p != NULL)
	{
		if (stage.note_scroll >= 0)
		{
			//Scroll through Pico chart
			u16 substep = stage.note_scroll >> FIXED_SHIFT;
			while (substep >= ((*this->pico_p) & 0x7FFF))
			{
				//Play animation and bump speakers
				character->set_anim(character, ((*this->pico_p) & 0x8000) ? CharAnim_RightAlt : CharAnim_LeftAlt);
				Speaker_Bump(&this->speaker);
				this->pico_p++;
			}
		}
	}
	else
	{
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
			//Stage specific animations
			if (stage.note_scroll >= 0)
			{
				switch (stage.stage_id)
				{
					case StageId_1_4: //Tutorial cheer
						if (stage.song_step > 64 && stage.song_step < 192 && (stage.song_step & 0x3F) == 60)
							character->set_anim(character, CharAnim_UpAlt);
						break;
					default:
						break;
				}
			}
			
			//Perform dance
			if (stage.note_scroll >= character->sing_end && (stage.song_step % stage.gf_speed) == 0)
			{
				//Switch animation
				if (character->animatable.anim == CharAnim_LeftAlt || character->animatable.anim == CharAnim_Right)
					character->set_anim(character, CharAnim_RightAlt);
				else
					character->set_anim(character, CharAnim_LeftAlt);
				
				//Bump speakers
				Speaker_Bump(&this->speaker);
			}
		}
	}
	
	//Get parallax
	fixed_t parallax;
	if (stage.stage_id >= StageId_1_1 && stage.stage_id <= StageId_1_4)
		parallax = FIXED_DEC(7,10);
	else
		parallax = FIXED_UNIT;
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_GF_SetFrame);
	Character_DrawParallax(character, &this->tex, &char_gf_frame[this->frame], parallax);
	
	//Tick speakers
	Speaker_Tick(&this->speaker, character->x, character->y, parallax);
}

void Char_GF_SetAnim(Character *character, u8 anim)
{
	//Set animation
	if (anim == CharAnim_Left || anim == CharAnim_Down || anim == CharAnim_Up || anim == CharAnim_Right || anim == CharAnim_UpAlt)
		character->sing_end = stage.note_scroll + FIXED_DEC(22,1); //Nearly 2 steps
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_GF_Free(Character *character)
{
	Char_GF *this = (Char_GF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_scene);
}

Character *Char_GF_New(fixed_t x, fixed_t y)
{
	//Allocate gf object
	Char_GF *this = Mem_Alloc(sizeof(Char_GF));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_GF_New] Failed to allocate gf object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_GF_Tick;
	this->character.set_anim = Char_GF_SetAnim;
	this->character.free = Char_GF_Free;
	
	Animatable_Init(&this->character.animatable, char_gf_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(2,1);
	this->character.focus_y = FIXED_DEC(-40,1);
	this->character.focus_zoom = FIXED_DEC(2,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\GF.ARC;1");
	
	const char **pathp = (const char *[]){
		"gf0.tim", //GF_ArcMain_GF0
		"gf1.tim", //GF_ArcMain_GF1
		"gf2.tim", //GF_ArcMain_GF2
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Load scene specific art
	switch (stage.stage_id)
	{
		case StageId_1_4: //Tutorial
		{
			this->arc_scene = IO_Read("\\CHAR\\GFTUT.ARC;1");
			
			const char **pathp = (const char *[]){
				"tut0.tim", //GF_ArcScene_0
				"tut1.tim", //GF_ArcScene_1
				NULL
			};
			IO_Data *arc_ptr = &this->arc_ptr[GF_ArcScene_0];
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_scene, *pathp);
			break;
		}
		default:
			this->arc_scene = NULL;
			break;
	}
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize speaker
	Speaker_Init(&this->speaker);
	
	//Initialize Pico test
	if (stage.stage_id == StageId_7_3 && stage.back != NULL)
		this->pico_p = ((Back_Week7*)stage.back)->pico_chart;
	else
		this->pico_p = NULL;
	
	return (Character*)this;
}
