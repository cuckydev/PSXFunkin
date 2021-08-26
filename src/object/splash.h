#ifndef _SPLASH_H
#define _SPLASH_H

#include "../object.h"
#include "../stage.h"

//Splash object structure
typedef struct
{
	//Object base structure
	Object obj;
	
	//Splash state
	u8 colour;
	
	fixed_t x, y, xsp, ysp, size, sin, cos;
} Obj_Splash;

//Splash object functions
Obj_Splash *Obj_Splash_New(fixed_t x, fixed_t y, u8 colour);

#endif
