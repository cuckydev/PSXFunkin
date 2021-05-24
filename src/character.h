#ifndef _CHARACTER_H
#define _CHARACTER_H

#include "io.h"
#include "gfx.h"

#include "fixed.h"
#include "animation.h"

//Character enums
typedef enum
{
	CharId_Dad,
	
	CharId_Max
} CharId;

typedef enum
{
	CharAnim_Idle,
	CharAnim_Left,  CharAnim_LeftAlt,
	CharAnim_Down,  CharAnim_DownAlt,
	CharAnim_Up,    CharAnim_UpAlt,
	CharAnim_Right, CharAnim_RightAlt,
	
	CharAnim_Max //Max standard/shared animation
} CharAnim;

//Character structures
typedef struct
{
	u8 tex;
	u16 src[4];
	s16 off[2];
} CharFrame;

/*
typedef struct
{
	u8 spd;
	const u8 *script;
} CharAnimDef;

typedef struct
{
	u8 tex;
	u16 src[4];
	s16 off[2];
} CharFrame;

typedef struct
{
	fixed_t focus_height;
	
	const char *arc_tex;
	u8 texs;
	const char **tex;
	
	const CharFrame *frame;
	
	const CharAnimDef anim[CharAnim_Max];
} CharDef;

typedef struct
{
	//Character textures and definition
	const CharDef *char_def;
	const CharAnimDef *anim_def;
	const CharFrame *frame_def;
	CharId char_id;
	
	IO_Data arc_tex;
	u8 texs;
	IO_Data *load_tex;
	
	//Character state
	fixed_t x, y;
	
	Gfx_Tex tex;
	u8 tex_i;
	
	CharAnim anim;
	const u8 *anim_p;
	u8 anim_spd, anim_time;
	
	u8 frame;
	u8 load_tex_i;
} Character;
*/

typedef struct Character
{
	//Character functions
	void (*tick)(struct Character*);
	void (*set_anim)(struct Character*, u8);
	void (*free)(struct Character*);
	
	//Position
	fixed_t x, y;
	fixed_t focus_height;
	
	//Animation state
	Animatable animatable;
} Character;

//Character functions
Character *Character_New(CharId id, fixed_t x, fixed_t y);
void Character_Free(Character *this);

void Character_Init(Character *this, fixed_t x, fixed_t y);
void Character_Draw(Character *this, Gfx_Tex *tex, const CharFrame *cframe);

#endif
