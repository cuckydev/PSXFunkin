#ifndef _SPEAKER_H
#define _SPEAKER_H

#include "../gfx.h"
#include "../fixed.h"

//Speaker structure
typedef struct
{
	//Speaker state
	Gfx_Tex tex;
	fixed_t bump;
} Speaker;

//Speaker functions
void Speaker_Init(Speaker *this);
void Speaker_Bump(Speaker *this);
void Speaker_Tick(Speaker *this, fixed_t x, fixed_t y);

#endif
