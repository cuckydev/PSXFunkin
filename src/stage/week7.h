/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _WEEK7_H
#define _WEEK7_H

#include "../stage.h"

//Week 7 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Car state
	fixed_t tank_x;
	fixed_t tank_timer;
	
	//Textures
	IO_Data arc_hench, arc_hench_ptr[2];
	
	Gfx_Tex tex_back0; //Foreground
	Gfx_Tex tex_back1; //Background
	Gfx_Tex tex_back2; //Sniper and Tank
	Gfx_Tex tex_back3; //Background mountains
	
	//Pico chart
	u16 *pico_chart;
} Back_Week7;

//Week 7 functions
StageBack *Back_Week7_New();

#endif
