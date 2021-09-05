/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

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
