#include "week4.h"

#include "../archive.h"
#include "../mem.h"

//Week 4 background functions
void Back_Week4_DrawBG(StageBack *back)
{
	Back_Week4 *this = (Back_Week4*)back;
	
	fixed_t fx, fy;
	
	//Draw foreground limo
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT fglimo_src = {0, 0, 256, 128};
	RECT_FIXED fglimo_dst = {
		FIXED_DEC(-220,1) - fx,
		FIXED_DEC(50,1) - fy,
		FIXED_DEC(256,1),
		FIXED_DEC(128,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &fglimo_src, &fglimo_dst, stage.bump);
	fglimo_dst.x += fglimo_dst.w;
	fglimo_dst.y -= (fglimo_dst.h * 22) >> 7;
	fglimo_src.y += 128;
	Stage_DrawTex(&this->tex_back0, &fglimo_src, &fglimo_dst, stage.bump);
	
	//Draw henchmen
	
	//Draw background limo
	fx = stage.camera.x / 3;
	fy = stage.camera.y / 3;
	
	RECT bglimo_src = {0, 0, 256, 128};
	RECT_FIXED bglimo_dst = {
		FIXED_DEC(-155,1) - fx,
		FIXED_DEC(30,1) - fy,
		FIXED_DEC(256,1),
		FIXED_DEC(128,1)
	};
	
	Stage_DrawTex(&this->tex_back1, &bglimo_src, &bglimo_dst, stage.bump);
	bglimo_dst.x += bglimo_dst.w;
	bglimo_src.y += 128;
	Stage_DrawTex(&this->tex_back1, &bglimo_src, &bglimo_dst, stage.bump);
	
	//Draw sunset
	fx = stage.camera.x / 8;
	fy = stage.camera.y / 8;
	
	RECT sunset_src = {0, 0, 256, 256};
	RECT_FIXED sunset_dst = {
		FIXED_DEC(-220,1) - fx,
		FIXED_DEC(-140,1) - fy,
		FIXED_DEC(400,1),
		FIXED_DEC(260,1)
	};
	
	Stage_DrawTex(&this->tex_back2, &sunset_src, &sunset_dst, stage.bump);
}

void Back_Week4_Free(StageBack *back)
{
	Back_Week4 *this = (Back_Week4*)back;
	
	//Free henchmen archive
	Mem_Free(this->arc_hench);
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week4_New()
{
	//Allocate background structure
	Back_Week4 *this = (Back_Week4*)Mem_Alloc(sizeof(Back_Week4));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week4_DrawBG;
	this->back.free = Back_Week4_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK4\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Mem_Free(arc_back);
	
	//Load henchmen textures
	this->arc_hench = IO_Read("\\WEEK4\\HENCH.ARC;1");
	this->arc_hench_ptr[0] = Archive_Find(this->arc_hench, "hench0.tim");
	this->arc_hench_ptr[1] = Archive_Find(this->arc_hench, "hench1.tim");
	
	return (StageBack*)this;
}
