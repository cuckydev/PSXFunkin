/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week6.h"

#include "../mem.h"

//Week 6 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
} Back_Week6;

//Week 6 background functions
void Back_Week6_Free(StageBack *back)
{
	Back_Week6 *this = (Back_Week6*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week6_New(void)
{
	//Allocate background structure
	Back_Week6 *this = (Back_Week6*)Mem_Alloc(sizeof(Back_Week6));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = NULL;
	this->back.free = Back_Week6_Free;
	
	return (StageBack*)this;
}
