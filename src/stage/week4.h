#ifndef _WEEK4_H
#define _WEEK4_H

#include "../stage.h"
#include "../gfx.h"

//Week 4 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	IO_Data arc_hench, arc_hench_ptr[2];
	
	Gfx_Tex tex_back0; //Foreground limo
	Gfx_Tex tex_back1; //Background limo
	Gfx_Tex tex_back2; //Sunset
	Gfx_Tex tex_back3; //Car
	
	Gfx_Tex tex_hench;
} Back_Week4;

//Week 4 functions
StageBack *Back_Week4_New();

#endif
