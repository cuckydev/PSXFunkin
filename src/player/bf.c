#include "bf.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Boyfriend player structureenum
enum
{
	BF_ArcMain_Idle,
	BF_ArcMain_Hit0,  //Left Down
	BF_ArcMain_Miss0, //Left Down
	BF_ArcMain_Hit1,  //Up Right
	BF_ArcMain_Miss1, //Up Right
	BF_ArcMain_Peace,
	BF_ArcMain_Dead,
	
	BF_ArcMain_Max,
};

typedef struct
{
	//Player base structure
	Player player;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[BF_ArcMain_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Player_BF;

//Boyfriend player definitions
static const CharFrame player_bf_frame[] = {
	{BF_ArcMain_Idle, {  0,   0, 128, 128}, { 53,  92}}, //0 idle 1
	{BF_ArcMain_Idle, {128,   0, 128, 128}, { 53,  93}}, //1 idle 2
	{BF_ArcMain_Idle, {  0, 128, 128, 128}, { 53,  98}}, //2 idle 3
	{BF_ArcMain_Idle, {128, 128, 128, 128}, { 53,  98}}, //3 idle 4
	
	{BF_ArcMain_Hit0,  {  0,   0, 128, 128}, { 56,  94}}, //4 left 1
	{BF_ArcMain_Hit0,  {128,   0, 128, 128}, { 56,  94}}, //5 left 2
	{BF_ArcMain_Miss0, {  0,   0, 128, 128}, { 52, 102}}, //6 left miss 1
	{BF_ArcMain_Miss0, {128,   0, 128, 128}, { 53, 102}}, //7 left miss 2
	
	{BF_ArcMain_Hit0,  {  0, 128, 128, 128}, { 50,  82}}, //8 down 1
	{BF_ArcMain_Hit0,  {128, 128, 128, 128}, { 50,  83}}, //9 down 2
	{BF_ArcMain_Miss0, {  0, 128, 128, 128}, { 49,  90}}, //10 down miss 1
	{BF_ArcMain_Miss0, {128, 128, 128, 128}, { 50,  90}}, //11 down miss 2
	
	{BF_ArcMain_Hit1,  {  0,   0, 128, 128}, { 42, 103}}, //12 up 1
	{BF_ArcMain_Hit1,  {128,   0, 128, 128}, { 44, 102}}, //13 up 2
	{BF_ArcMain_Miss1, {  0,   0, 128, 128}, { 48,  99}}, //14 up miss 1
	{BF_ArcMain_Miss1, {128,   0, 128, 128}, { 48, 100}}, //15 up miss 2
	
	{BF_ArcMain_Hit1,  {  0, 128, 128, 128}, { 42,  94}}, //16 right 1
	{BF_ArcMain_Hit1,  {128, 128, 128, 128}, { 42,  95}}, //17 right 2
	{BF_ArcMain_Miss1, {  0, 128, 128, 128}, { 45, 102}}, //18 right miss 1
	{BF_ArcMain_Miss1, {128, 128, 128, 128}, { 43, 102}}, //19 right miss 2
	
	{BF_ArcMain_Peace, {  0,   0, 128, 128}, { 53,  98}}, //20 peace 1
	{BF_ArcMain_Peace, {128,   0, 128, 128}, { 53,  97}}, //21 peace 2
	{BF_ArcMain_Peace, {  0, 128, 128, 128}, { 53,  97}}, //22 peace 3
};

static const Animation player_bf_anim[PlayerAnim_Max] = {
	{4, (const u8[]){ 0,  1,  2,  3,  ASCR_BACK, 1}},                                          //CharAnim_Idle
	{2, (const u8[]){ 4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Left
	{2, (const u8[]){ 4,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_LeftAlt
	{2, (const u8[]){ 8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Down
	{2, (const u8[]){ 8, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_DownAlt
	{2, (const u8[]){12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Up
	{2, (const u8[]){12, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_UpAlt
	{2, (const u8[]){16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Right
	{2, (const u8[]){16, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_RightAlt
	{3, (const u8[]){20, 21, 22, 22, 22, 22, 22, 22, 22, ASCR_CHGANI, CharAnim_Idle}},         //PlayerAnim_Peace
	{3, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                                             //PlayerAnim_Sweat
};

//Boyfriend player functions
void Player_BF_SetFrame(void *user, u8 frame)
{
	Player_BF *this = (Player_BF*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &player_bf_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Player_BF_Tick(Character *character)
{
	Player_BF *this = (Player_BF*)character;
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Player_BF_SetFrame);
	Character_Draw(character, &this->tex, &player_bf_frame[this->frame]);
}

void Player_BF_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
}

void Player_BF_Free(Character *character)
{
	Player_BF *this = (Player_BF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Player *Player_BF_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Player_BF *this = Mem_Alloc(sizeof(Player_BF));
	if (this == NULL)
	{
		sprintf(error_msg, "[Player_BF_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->player.character.tick = Player_BF_Tick;
	this->player.character.set_anim = Player_BF_SetAnim;
	this->player.character.free = Player_BF_Free;
	
	Animatable_Init(&this->player.character.animatable, player_bf_anim);
	Character_Init((Character*)this, x, y);
	
	this->player.character.focus_height = FIXED_DEC(32,1);
	
	//Load art
	this->arc_main = IO_Read("\\BF\\MAIN.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle.tim",  //BF_ArcMain_Idle
		"hit0.tim",  //BF_ArcMain_Hit0
		"miss0.tim", //BF_ArcMain_Miss0
		"hit1.tim",  //BF_ArcMain_Hit1
		"miss1.tim", //BF_ArcMain_Miss1
		"peace.tim", //BF_ArcMain_Peace
		"dead0.tim", //BF_ArcMain_Dead
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (u8 i = 0; i < BF_ArcMain_Max; i++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp++);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Player*)this;
}
