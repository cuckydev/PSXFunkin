#include "gf.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//GF character structure
enum
{
	GF_ArcMain_BopLeft,
	GF_ArcMain_BopRight,
	GF_ArcMain_Cry,
	
	GF_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[GF_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_GF;

//GF character definitions
static const CharFrame char_gf_frame[] = {
	{GF_ArcMain_BopLeft, {  0,   0,  74, 103}, { 40,  74}}, //0 bop left 1
	{GF_ArcMain_BopLeft, { 74,   0,  73, 102}, { 39,  74}}, //1 bop left 2
	{GF_ArcMain_BopLeft, {147,   0,  73, 102}, { 39,  74}}, //2 bop left 3
	{GF_ArcMain_BopLeft, {  0, 103,  73, 103}, { 39,  75}}, //3 bop left 4
	{GF_ArcMain_BopLeft, { 73, 102,  82, 105}, { 43,  77}}, //4 bop left 5
	{GF_ArcMain_BopLeft, {155, 102,  81, 105}, { 43,  77}}, //5 bop left 6
	
	{GF_ArcMain_BopRight, {  0,   0,  81, 103}, { 43,  75}}, //6 bop right 1
	{GF_ArcMain_BopRight, { 81,   0,  81, 103}, { 43,  75}}, //7 bop right 2
	{GF_ArcMain_BopRight, {162,   0,  80, 103}, { 42,  75}}, //8 bop right 3
	{GF_ArcMain_BopRight, {  0, 103,  79, 103}, { 41,  75}}, //9 bop right 4
	{GF_ArcMain_BopRight, { 79, 103,  73, 105}, { 35,  77}}, //10 bop right 5
	{GF_ArcMain_BopRight, {152, 103,  74, 104}, { 35,  76}}, //11 bop right 6
	
	{GF_ArcMain_Cry, {  0,   0,  73, 101}, { 37,  74}}, //12 cry
	{GF_ArcMain_Cry, { 73,   0,  73, 101}, { 37,  74}}, //13 cry
};

static const Animation char_gf_anim[CharAnim_Max] = {
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                                           //CharAnim_Idle
	{1, (const u8[]){ 0,  0,  1,  1,  2,  2,  3,  3,  3,  4,  4,  4,  4,  5, ASCR_BACK, 1}}, //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                                           //CharAnim_LeftAlt
	{2, (const u8[]){12, 13, ASCR_REPEAT}},                                                  //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                                           //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                                           //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                                           //CharAnim_UpAlt
	{1, (const u8[]){ 6,  6,  7,  7,  8,  8,  9,  9,  9, 10, 10, 10, 10, 11, ASCR_BACK, 1}}, //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Left}},                                           //CharAnim_RightAlt
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
	
	if (stage.just_step)
	{
		//Perform dance
		if ((stage.song_step % stage.gf_speed) == 0)
		{
			if (character->animatable.anim == CharAnim_Left)
				character->set_anim(character, CharAnim_Right);
			else
				character->set_anim(character, CharAnim_Left);
		}
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_GF_SetFrame);
	Character_Draw(character, &this->tex, &char_gf_frame[this->frame]);
}

void Char_GF_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_GF_Free(Character *character)
{
	Char_GF *this = (Char_GF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
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
	
	//Set character stage information
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(16,1);
	this->character.focus_y = FIXED_DEC(-50,1);
	this->character.focus_zoom = FIXED_DEC(15,10);
	
	//Load art
	this->arc_main = IO_Read("\\GF\\MAIN.ARC;1");
	
	const char **pathp = (const char *[]){
		"bopleft.tim",  //GF_ArcMain_BopLeft
		"bopright.tim", //GF_ArcMain_BopRight
		"cry.tim",      //GF_ArcMain_Cry
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (u8 i = 0; i < GF_Arc_Max; i++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp++);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
