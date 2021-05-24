#include "character.h"

#include "mem.h"
#include "stage.h"

//Characters
#include "character/dad.h"

static Character* (*char_new[CharId_Max])(fixed_t, fixed_t) = {
	Char_Dad_New,
};

//Character functions
Character *Character_New(CharId id, fixed_t x, fixed_t y)
{
	//Allocate new character from given id
	return char_new[id](x, y);
}

void Character_Free(Character *this)
{
	//Free character
	this->free(this);
	Mem_Free(this);
}

void Character_Init(Character *this, fixed_t x, fixed_t y)
{
	//Perform common character initialization
	this->x = x;
	this->y = y;
	
	this->set_anim(this, CharAnim_Idle);
}

void Character_Draw(Character *this, Gfx_Tex *tex, const CharFrame *cframe)
{
	//Draw character
	fixed_t x = this->x - stage.camera.x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t y = this->y - stage.camera.y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {x, y, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(tex, &src, &dst, FIXED_MUL(stage.camera.zoom, stage.bump));
}
