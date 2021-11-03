/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "animation.h"

#include "timer.h"

//Animation functions
void Animatable_Init(Animatable *this, const Animation *anims)
{
	//Use given anims array
	this->anims = anims;
}

void Animatable_SetAnim(Animatable *this, u8 anim)
{
	//Start animation
	this->anim = anim;
	this->anim_p = this->anims[anim].script;
	this->anim_spd = FIXED_DEC(this->anims[anim].spd,1) / 24;
	this->anim_time = 0;
	this->ended = false;
}

void Animatable_Animate(Animatable *this, void *user, void (*set_frame)(void*, u8))
{
	//Wait for time
	while (this->anim_time <= 0)
	{
		//Read script
		switch (this->anim_p[0])
		{
			case ASCR_REPEAT:
				this->anim_p = this->anims[this->anim].script;
				this->ended = true;
				break;
			case ASCR_CHGANI:
				Animatable_SetAnim(this, this->anim_p[1]);
				break;
			case ASCR_BACK:
				this->anim_time += this->anim_spd;
				this->anim_p -= this->anim_p[1];
				this->ended = true;
				break;
			default:
				set_frame(user, this->anim_p[0]);
				this->anim_time += this->anim_spd;
				this->anim_p++;
				break;
		}
	}
	this->anim_time -= timer_dt;
}

boolean Animatable_Ended(Animatable *this)
{
	return this->ended;
}
