#ifndef _WEEK4_H
#define _WEEK4_H

#include "../stage.h"
#include "../gfx.h"
#include "../animation.h"

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
	
	//Car state
	fixed_t car_x;
	u16 car_timer;
	
	//Henchmen state
	Gfx_Tex tex_hench;
	u8 hench_frame, hench_tex_id;
	
	Animatable hench_animatable;
} Back_Week4;

//Week 4 functions
StageBack *Back_Week4_New();

#endif
