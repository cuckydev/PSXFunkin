#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "psx.h"

//Animation structures
#define ASCR_REPEAT 0xFF
#define ASCR_CHGANI 0xFE
#define ASCR_BACK   0xFD

typedef struct
{
	//Animation data and script
	u8 spd;
	const u8 *script;
} Animation;

typedef struct
{
	//Animation state
	const Animation *anims;
	const u8 *anim_p;
	u8 anim, anim_spd, anim_time;
} Animatable;

//Animation functions
void Animatable_Init(Animatable *this, const Animation *anims);
void Animatable_SetAnim(Animatable *this, u8 anim);
void Animatable_Animate(Animatable *this, void *user, void (*set_frame)(void*, u8));

#endif
