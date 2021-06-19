#ifndef _DUMMY_H
#define _DUMMY_H

#include "../stage.h"

//Dummy background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
} Back_Dummy;

//Dummy functions
StageBack *Back_Dummy_New();

#endif
