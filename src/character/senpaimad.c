#include "senpaimad.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Dad character structure
enum
{
	Senpaimad_ArcMain_Senpaimad0,
	Senpaimad_ArcMain_Senpaimad1,
	
	Senpaimad_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Senpaimad_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Senpaimad;

//Dad character definitions
static const CharFrame char_senpaimad_frame[] = {
	{Senpaimad_ArcMain_Senpaimad0, {  0,   0,  66, 112}, { 32, 107}}, //0 idle 1
	{Senpaimad_ArcMain_Senpaimad0, { 66,   0,  65, 112}, { 31, 107}}, //1 idle 2
	{Senpaimad_ArcMain_Senpaimad0, {131,   0,  65, 113}, { 31, 108}}, //2 idle 3
	{Senpaimad_ArcMain_Senpaimad0, {  0, 112,  67, 116}, { 31, 111}}, //3 idle 4
	{Senpaimad_ArcMain_Senpaimad0, { 71, 112,  67, 115}, { 31, 110}}, //4 idle 5
	
	{Senpaimad_ArcMain_Senpaimad1, {  0,   0,  59, 115}, { 33, 110}}, //5 left 1
	{Senpaimad_ArcMain_Senpaimad1, { 59,   0,  61, 115}, { 35, 110}}, //6 left 2
	
	{Senpaimad_ArcMain_Senpaimad1, {120,   0,  63, 107}, { 31, 102}}, //7 down 1
	{Senpaimad_ArcMain_Senpaimad1, {183,   1,  62, 108}, { 30, 103}}, //8 down 2
	
	{Senpaimad_ArcMain_Senpaimad1, {  0, 115,  64, 122}, { 31, 115}}, //9 up 1
	{Senpaimad_ArcMain_Senpaimad1, { 64, 115,  66, 122}, { 32, 115}}, //10 up 2
	
	{Senpaimad_ArcMain_Senpaimad1, {130, 109,  65, 114}, { 28, 109}}, //11 right 1
	{Senpaimad_ArcMain_Senpaimad0, {192, 140,  64, 114}, { 27, 109}}, //12 right 2
};

static const Animation char_senpaimad_anim[CharAnim_Max] = {
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
void Char_Senpaimad_SetFrame(void *user, u8 frame)
{
	Char_Senpaimad *this = (Char_Senpaimad*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_senpaimad_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Senpaimad_Tick(Character *character)
{
	Char_Senpaimad *this = (Char_Senpaimad*)character;
	
	//Perform idle dance
	Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		if (Animatable_Ended(&character->animatable) &&
		    (character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_Right) &&
		    (stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Senpaimad_SetFrame);
	Character_Draw(character, &this->tex, &char_senpaimad_frame[this->frame]);
}

void Char_Senpaimad_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Senpaimad_Free(Character *character)
{
	Char_Senpaimad *this = (Char_Senpaimad*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Senpaimad_New(fixed_t x, fixed_t y)
{
	//Allocate senpaimad object
	Char_Senpaimad *this = Mem_Alloc(sizeof(Char_Senpaimad));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Senpaimad_New] Failed to allocate senpaimad object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Senpaimad_Tick;
	this->character.set_anim = Char_Senpaimad_SetAnim;
	this->character.free = Char_Senpaimad_Free;
	
	Animatable_Init(&this->character.animatable, char_senpaimad_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character stage information
	this->character.health_i = 8;
	
	this->character.focus_x = FIXED_DEC(32,1);
	this->character.focus_y = FIXED_DEC(-58,1);
	this->character.focus_zoom = FIXED_DEC(2,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SENPAImad.ARC;1");
	
	const char **pathp = (const char *[]){
		"senpaimad0.tim", //Senpaimad_ArcMain_Senpaimad0
		"senpaimad1.tim", //Senpaimad_ArcMain_Senpaimad1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
