/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "tank.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"
#include "../timer.h"

//Tank character structure
enum
{
	Tank_ArcMain_Idle0,
	Tank_ArcMain_Idle1,
	Tank_ArcMain_Left,
	Tank_ArcMain_Down,
	Tank_ArcMain_Up,
	Tank_ArcMain_Right,
	
	Tank_ArcScene_0, //ugh0 good0
	Tank_ArcScene_1, //ugh1 good1
	Tank_ArcScene_2, //good2
	Tank_ArcScene_3, //good3
	
	Tank_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_scene;
	IO_Data arc_ptr[Tank_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
	//Mouth state
	fixedu_t mouth_i;
} Char_Tank;

//Tank character definitions
static const CharFrame char_tank_frame[] = {
	{Tank_ArcMain_Idle0, {  0,   0, 128, 256}, { 51, 125}}, //0 idle 1
	{Tank_ArcMain_Idle0, {128,   0, 128, 256}, { 59, 126}}, //1 idle 2
	{Tank_ArcMain_Idle1, {  0,   0, 128, 256}, { 52, 130}}, //2 idle 3
	{Tank_ArcMain_Idle1, {128,   0, 128, 256}, { 53, 129}}, //3 idle 4
	
	{Tank_ArcMain_Left, {  0,   0, 128, 256}, { 75, 126}}, //4 left 1
	{Tank_ArcMain_Left, {128,   0, 128, 256}, { 72, 127}}, //5 left 1
	
	{Tank_ArcMain_Down, {  0,   0, 256, 128}, { 68, 100}}, //6 down 1
	{Tank_ArcMain_Down, {  0, 128, 256, 128}, { 68, 102}}, //7 down 2
	
	{Tank_ArcMain_Up, {  0,   0, 128, 256}, { 62, 144}}, //8 up 1
	{Tank_ArcMain_Up, {128,   0, 128, 256}, { 66, 142}}, //9 up 2
	
	{Tank_ArcMain_Right, {  0,   0, 128, 256}, { 48, 119}}, //10 right 1
	{Tank_ArcMain_Right, {128,   0, 128, 256}, { 46, 123}}, //11 right 2
	
	{Tank_ArcScene_0, {  0,   0, 128, 256}, { 48, 125}}, //12 ugh 0
	{Tank_ArcScene_0, {128,   0, 128, 256}, { 50, 128}}, //13 ugh 1
	{Tank_ArcScene_1, {  0,   0, 128, 256}, { 49, 128}}, //14 ugh 2
	{Tank_ArcScene_1, {128,   0, 128, 256}, { 49, 128}}, //15 ugh 3
	
	
	{Tank_ArcScene_0, {  0,   0, 128, 256}, { 53, 128}}, //16 good 0
	{Tank_ArcScene_0, {128,   0, 128, 256}, { 53, 133}}, //17 good 1
	{Tank_ArcScene_1, {  0,   0, 128, 256}, { 53, 131}}, //18 good 2
	{Tank_ArcScene_1, {128,   0, 128, 256}, { 53, 131}}, //19 good 3
	{Tank_ArcScene_2, {  0,   0, 128, 256}, { 53, 131}}, //20 good 4
	{Tank_ArcScene_2, {128,   0, 128, 256}, { 52, 126}}, //21 good 5
	{Tank_ArcScene_3, {  0,   0, 132, 256}, { 53, 127}}, //22 good 6
};

static const Animation char_tank_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3, ASCR_BACK, 1}},                                           //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},                                                   //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                                             //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},                                                   //CharAnim_Down
	{2, (const u8[]){16, 17, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,   //CharAnim_DownAlt
	                 19, 20, 21, 22, ASCR_BACK, 1}},
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},                                                   //CharAnim_Up
	{2, (const u8[]){12, 13, 14, 15, ASCR_BACK, 1}},                                           //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},                                                   //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                                             //CharAnim_RightAlt
};

//Tank character functions
void Char_Tank_SetFrame(void *user, u8 frame)
{
	Char_Tank *this = (Char_Tank*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_tank_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Tank_Tick(Character *character)
{
	Char_Tank *this = (Char_Tank*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 &&
	     character->animatable.anim != CharAnim_DownAlt) //Don't interrupt "Heh, pretty good!" sequence
		Character_PerformIdle(character);
	
	//Animate
	Animatable_Animate(&character->animatable, (void*)this, Char_Tank_SetFrame);
	
	//Draw mouth if saying "...pretty good!"
	if (this->frame == 22)
	{
		//Mouth mappings
		static const u8 mouth_map[] = {
			0, 1, 1, 2, 2, 2,       //etty
			4, 4, 5, 5, 6, 6, 7, 7, //good!
		};
		
		//Get mouth frame
		u8 mouth_frame = mouth_map[(this->mouth_i * 24) >> FIXED_SHIFT];
		
		this->mouth_i += timer_dt;
		if ((this->mouth_i * 24) >= (COUNT_OF(mouth_map) << FIXED_SHIFT))
			this->mouth_i = ((COUNT_OF(mouth_map) - 1) << FIXED_SHIFT) / 24;
		
		//Draw mouth
		RECT mouth_src = {
			144 + (mouth_frame & 3) * 24,
			(mouth_frame >> 2) << 4,
			24,
			16
		};
		RECT_FIXED mouth_dst = {
			character->x - FIXED_DEC(12,1) - stage.camera.x,
			character->y - FIXED_DEC(84,1) - stage.camera.y,
			FIXED_DEC(24,1),
			FIXED_DEC(16,1)
		};
		
		Stage_DrawTex(&this->tex, &mouth_src, &mouth_dst, stage.camera.bzoom);
	}
	
	//Draw body
	Character_Draw(character, &this->tex, &char_tank_frame[this->frame]);
}

void Char_Tank_SetAnim(Character *character, u8 anim)
{
	//Set animation
	if (anim == CharAnim_DownAlt && character->animatable.anim != CharAnim_DownAlt)
	{
		character->focus_x = FIXED_DEC(120,1);
		character->focus_y = FIXED_DEC(-100,1);
		character->focus_zoom = FIXED_DEC(78, 100);
	}
	else if (anim != CharAnim_DownAlt && character->animatable.anim == CharAnim_DownAlt)
	{
		character->focus_x = FIXED_DEC(65,1);
		character->focus_y = FIXED_DEC(-80,1);
		character->focus_zoom = FIXED_UNIT;
	}
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Tank_Free(Character *character)
{
	Char_Tank *this = (Char_Tank*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_scene);
}

Character *Char_Tank_New(fixed_t x, fixed_t y)
{
	//Allocate tank object
	Char_Tank *this = Mem_Alloc(sizeof(Char_Tank));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Tank_New] Failed to allocate tank object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Tank_Tick;
	this->character.set_anim = Char_Tank_SetAnim;
	this->character.free = Char_Tank_Free;
	
	Animatable_Init(&this->character.animatable, char_tank_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 10;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-80,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\TANK.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Tank_ArcMain_Idle0
		"idle1.tim", //Tank_ArcMain_Idle1
		"left.tim",  //Tank_ArcMain_Left
		"down.tim",  //Tank_ArcMain_Down
		"up.tim",    //Tank_ArcMain_Up
		"right.tim", //Tank_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Load scene art
	switch (stage.stage_id)
	{
		case StageId_7_1: //Ugh
		{
			//Load "Ugh" art
			this->arc_scene = IO_Read("\\CHAR\\TANKUGH.ARC;1");
			
			const char **pathp = (const char *[]){
				"ugh0.tim", //Tank_ArcScene_0
				"ugh1.tim", //Tank_ArcScene_1
				NULL
			};
			IO_Data *arc_ptr = &this->arc_ptr[Tank_ArcScene_0];
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_scene, *pathp);
			break;
		}
		case StageId_7_3: //Stress
		{
			//Load "Heh, pretty good!" art
			this->arc_scene = IO_Read("\\CHAR\\TANKGOOD.ARC;1");
			
			const char **pathp = (const char *[]){
				"good0.tim", //Tank_ArcScene_0
				"good1.tim", //Tank_ArcScene_1
				"good2.tim", //Tank_ArcScene_2
				"good3.tim", //Tank_ArcScene_3
				NULL
			};
			IO_Data *arc_ptr = &this->arc_ptr[Tank_ArcScene_0];
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_scene, *pathp);
			
			this->mouth_i = 0;
			break;
		}
		default:
		{
			this->arc_scene = NULL;
			break;
		}
	}
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
