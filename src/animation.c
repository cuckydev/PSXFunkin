#include "animation.h"

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
	this->anim_time = 0;
	this->anim_spd = this->anims[anim].spd;
	this->ended = false;
}

void Animatable_Animate(Animatable *this, void *user, void (*set_frame)(void*, u8))
{
	//Wait for time
	if (this->anim_time)
	{
		this->anim_time--;
		return;
	}
	
	while (1)
	{
		//Read script
		switch (this->anim_p[0])
		{
			case ASCR_REPEAT:
				this->anim_p = this->anims[this->anim].script;
				this->ended = true;
				break;
			case ASCR_CHGANI:
				this->anim_p = this->anims[this->anim = this->anim_p[1]].script;
				break;
			case ASCR_BACK:
				this->anim_time = this->anims[this->anim].spd;
				this->anim_p -= this->anim_p[1];
				this->ended = true;
				break;
			default:
				set_frame(user, this->anim_p[0]);
				this->anim_time = this->anims[this->anim].spd;
				this->anim_p++;
				return;
		}
	}
}

boolean Animatable_Ended(Animatable *this)
{
	return this->ended;
}
