#ifndef _WEEK7_H
#define _WEEK7_H

#include "../stage.h"
#include "../gfx.h"

//Week 7 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Car state
	fixed_t tank_x;
	u8 tank_i;
	u16 tank_timer;
	
	//Textures
	IO_Data arc_hench, arc_hench_ptr[2];
	
	Gfx_Tex tex_back0; //Foreground
	Gfx_Tex tex_back1; //Background
	Gfx_Tex tex_back2; //Sniper and Tank
	Gfx_Tex tex_back3; //Background mountains
} Back_Week7;

//Week 7 functions
StageBack *Back_Week7_New();

#endif
