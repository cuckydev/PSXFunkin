#include "week3.h"

#include "../mem.h"
#include "../archive.h"

//Week 3 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //Buildings
	Gfx_Tex tex_back1; //Lights
	Gfx_Tex tex_back2; //Rooftop
	Gfx_Tex tex_back3; //Background train arc
	Gfx_Tex tex_back4; //Train
	Gfx_Tex tex_back5; //Sky
} Back_Week3;

#include "pad.h"
void RectControl(RECT_FIXED *rect)
{
	if (pad_state.held & PAD_TRIANGLE)
	{
		if (pad_state.held & PAD_LEFT)
			rect->w -= FIXED_DEC(1,1);
		if (pad_state.held & PAD_UP)
			rect->h -= FIXED_DEC(1,1);
		if (pad_state.held & PAD_RIGHT)
			rect->w += FIXED_DEC(1,1);
		if (pad_state.held & PAD_DOWN)
			rect->h += FIXED_DEC(1,1);
	}
	else
	{
		if (pad_state.held & PAD_LEFT)
			rect->x -= FIXED_DEC(1,1);
		if (pad_state.held & PAD_UP)
			rect->y -= FIXED_DEC(1,1);
		if (pad_state.held & PAD_RIGHT)
			rect->x += FIXED_DEC(1,1);
		if (pad_state.held & PAD_DOWN)
			rect->y += FIXED_DEC(1,1);
	}
	FntPrint("%d %d %d %d", rect->x >> FIXED_SHIFT, rect->y >> FIXED_SHIFT, rect->w >> FIXED_SHIFT, rect->h >> FIXED_SHIFT);
}

//Week 3 background functions
void Back_Week3_DrawBG(StageBack *back)
{
	Back_Week3 *this = (Back_Week3*)back;
	
	fixed_t fx, fy;
	
	//Draw rooftop
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	static const RECT roof_src[] = {
		{  0, 0,  16, 256},
		{ 16, 0,  55, 256},
		{ 71, 0, 128, 256},
		{199, 0,  55, 256},
		{255, 0,   0, 256}
	};
	static const fixed_t roof_sc[] = {
		FIXED_DEC(3,1)*7/10,
		FIXED_DEC(1,1)*9/10,
		FIXED_DEC(265,100)*7/10,
		FIXED_DEC(1,1)*9/10,
		FIXED_DEC(16,1)
	};
	
	RECT_FIXED roof_dst = {
		FIXED_DEC(-210,1) - fx,
		FIXED_DEC(-106,1) - fy,
		0,
		FIXED_DEC(220,1)
	};
	
	const RECT *roof_srcp = roof_src;
	const fixed_t *roof_scp = roof_sc;
	
	for (int i = 0; i < COUNT_OF(roof_src); i++, roof_srcp++, roof_scp++)
	{
		roof_dst.w = (roof_srcp->w ? roof_srcp->w : 1) * *roof_scp;
		Stage_DrawTex(&this->tex_back2, roof_srcp, &roof_dst, stage.camera.bzoom);
		roof_dst.x += roof_dst.w;
	}
	
	RECT roof_fill = {0, SCREEN_HEIGHT * 2 / 3, SCREEN_WIDTH, SCREEN_HEIGHT * 1 / 3};
	Gfx_DrawRect(&roof_fill, 49, 58, 115);
	
	//Draw train
	
	//Draw arc
	RECT arcl_src = {0, 0, 38, 121};
	RECT_FIXED arcl_dst = {
		FIXED_DEC(-131,1) - fx,
		FIXED_DEC(-86,1) - fy,
		FIXED_DEC(38,1),
		FIXED_DEC(121,1)
	};
	
	RECT arcr_src = {39, 0, 39, 121};
	RECT_FIXED arcr_dst = {
		FIXED_DEC(74,1) - fx,
		FIXED_DEC(-85,1) - fy,
		FIXED_DEC(39,1),
		FIXED_DEC(121,1)
	};
	
	Stage_DrawTex(&this->tex_back3, &arcl_src, &arcl_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back3, &arcr_src, &arcr_dst, stage.camera.bzoom);
	
	//Draw lights
	fx = stage.camera.x >> 1;
	fy = stage.camera.y >> 1;
	
	RECT lightl_src = {0, 0, 256, 132};
	RECT_FIXED lightl_dst = {
		FIXED_DEC(-175,1) - fx,
		FIXED_DEC(-80,1) - fy,
		FIXED_DEC(195,1),
		FIXED_DEC(103,1)
	};
	
	RECT lightr_src = {0, 132, 256, 124};
	RECT_FIXED lightr_dst = {
		FIXED_DEC(98,1) - fx,
		FIXED_DEC(-64,1) - fy,
		FIXED_DEC(198,1),
		FIXED_DEC(95,1)
	};
	
	Stage_DrawTex(&this->tex_back1, &lightl_src, &lightl_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back1, &lightr_src, &lightr_dst, stage.camera.bzoom);
	
	//Draw buildings
	RECT building_src = {0, 0, 255, 128};
	RECT_FIXED building_dst = {
		FIXED_DEC(-195,1) - fx,
		FIXED_DEC(-120,1) - fy,
		FIXED_DEC(240,1),
		FIXED_DEC(120,1)
	};
	
	Stage_DrawTex(&this->tex_back0, &building_src, &building_dst, stage.camera.bzoom);
	building_dst.x += building_dst.w;
	building_src.y += building_src.h;
	Stage_DrawTex(&this->tex_back0, &building_src, &building_dst, stage.camera.bzoom);
	
	RECT building_fill = {0, SCREEN_HEIGHT * 3 / 7, SCREEN_WIDTH, SCREEN_HEIGHT * 4 / 7};
	Gfx_DrawRect(&building_fill, 16, 8, 25);
	
	//Draw sky
	fx = stage.camera.x >> 3;
	fy = stage.camera.y >> 3;
	
	RECT sky_src = {0, 0, 255, 128};
	RECT_FIXED sky_dst = {
		FIXED_DEC(-166,1) - fx,
		FIXED_DEC(-117,1) - fy,
		FIXED_DEC(172,1),
		FIXED_DEC(110,1)
	};
	
	Stage_DrawTex(&this->tex_back5, &sky_src, &sky_dst, stage.camera.bzoom);
	sky_dst.x += sky_dst.w;
	sky_src.y += sky_src.h;
	Stage_DrawTex(&this->tex_back5, &sky_src, &sky_dst, stage.camera.bzoom);
}

void Back_Week3_Free(StageBack *back)
{
	Back_Week3 *this = (Back_Week3*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week3_New()
{
	//Allocate background structure
	Back_Week3 *this = (Back_Week3*)Mem_Alloc(sizeof(Back_Week3));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week3_DrawBG;
	this->back.free = Back_Week3_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK3\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Gfx_LoadTex(&this->tex_back4, Archive_Find(arc_back, "back4.tim"), 0);
	Gfx_LoadTex(&this->tex_back5, Archive_Find(arc_back, "back5.tim"), 0);
	Mem_Free(arc_back);
	
	//Use sky background colour
	//Gfx_SetClear(148, 25, 99);
	
	return (StageBack*)this;
}
