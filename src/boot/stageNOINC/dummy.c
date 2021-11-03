/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "dummy.h"

#include "../mem.h"

//Dummy background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
} Back_Dummy;

//Dummy background functions
void Back_Dummy_Free(StageBack *back)
{
	Back_Dummy *this = (Back_Dummy*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Dummy_New(void)
{
	//Allocate background structure
	Back_Dummy *this = (Back_Dummy*)Mem_Alloc(sizeof(Back_Dummy));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = NULL;
	this->back.free = Back_Dummy_Free;
	
	//Use non-pitch black background
	Gfx_SetClear(62, 48, 64);
	
	return (StageBack*)this;
}
