#include "dummy.h"

#include "../mem.h"

//Dummy background functions
void Back_Dummy_Free(StageBack *back)
{
	Back_Dummy *this = (Back_Dummy*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Dummy_New()
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
	
	return (StageBack*)this;
}
