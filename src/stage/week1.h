#ifndef _WEEK1_H
#define _WEEK1_H

#include "../stage.h"

//Week 1 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //Stage and back
	Gfx_Tex tex_back1; //Curtains
} Back_Week1;

//Week 1 functions
StageBack *Back_Week1_New();

#endif
