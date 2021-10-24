/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_ANIMATION_H
#define PSXF_GUARD_ANIMATION_H

#include "psx.h"

#include "fixed.h"

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
	u8 anim;
	fixed_t anim_time, anim_spd;
	boolean ended;
} Animatable;

//Animation functions
void Animatable_Init(Animatable *this, const Animation *anims);
void Animatable_SetAnim(Animatable *this, u8 anim);
void Animatable_Animate(Animatable *this, void *user, void (*set_frame)(void*, u8));
boolean Animatable_Ended(Animatable *this);

#endif
