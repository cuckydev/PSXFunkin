#include "psx.h"

#include "mem.h"
#include "gfx.h"
#include "main.h"

//Gfx constants
#define OTLEN 8

//Gfx state
static DISPENV disp[2];
static DRAWENV draw[2];
static u8 db;

static u32 ot[2][OTLEN];    //Ordering table length
static u8 pribuff[2][32768]; //Primitive buffer
static u8 *nextpri;          //Next primitive pointer

//Gfx functions
void Gfx_Init()
{
	//Reset GPU
	ResetGraph(0);
	
	//Initialize display environment
	SetDefDispEnv(&disp[0], 0, 0, 320, 240);
	SetDefDispEnv(&disp[1], 0, 240, 320, 240);
	
	//Initialize draw environment
	SetDefDrawEnv(&draw[0], 0, 240, 320, 240);
	SetDefDrawEnv(&draw[1], 0, 0, 320, 240);
	
	//Set video mode depending on BIOS region
	switch(*(char *)0xbfc7ff52)
	{
		case 'E':
			SetVideoMode(MODE_PAL);
			SsSetTickMode(SS_TICK50);
			break;
		default:
			SetVideoMode(MODE_NTSC);
			SsSetTickMode(SS_TICK60);
			break;
	}
	
	//Set draw background
	draw[0].isbg = draw[1].isbg = 1;
	setRGB0(&draw[0], 0, 0, 0);
	setRGB0(&draw[1], 0, 0, 0);
	
	//Load font
	FntLoad(960, 0);
	FntOpen(0, 8, 320, 224, 0, 100);
	
	//Initialize drawing state
	nextpri = pribuff[0];
	db = 0;
	Gfx_Flip();
	Gfx_Flip();
}

void Gfx_Flip()
{
	//Sync
	DrawSync(0);
	VSync(0);
	
	//Apply environments
	PutDispEnv(&disp[db]);
	PutDrawEnv(&draw[db]);
	
	//Enable display
	SetDispMask(1);
	
	//Draw screen
	DrawOTag(ot[db] + OTLEN - 1);
	FntFlush(-1);
	
	//Flip buffers
	db ^= 1;
	nextpri = pribuff[db];
	ClearOTagR(ot[db], OTLEN);
}

void Gfx_SetClear(u8 r, u8 g, u8 b)
{
	setRGB0(&draw[0], r, g, b);
    setRGB0(&draw[1], r, g, b);
}

void Gfx_LoadTex(Gfx_Tex *tex, IO_Data data, Gfx_LoadTex_Flag flag)
{
	//Catch NULL data
	if (data == NULL)
	{
		sprintf(error_msg, "[Gfx_LoadTex] data is NULL");
		ErrorLock();
	}
	
	//Read TIM information
	TIM_IMAGE tparam;
	OpenTIM(data);
	ReadTIM(&tparam);
	
	if (tex != NULL)
	{
		tex->tim_mode = tparam.mode;
		tex->pxshift = (2 - (tparam.mode & 0x3));
	}
	
	//Upload pixel data to framebuffer
	if (!(flag & GFX_LOADTEX_NOTEX))
	{
		if (tex != NULL)
		{
			memcpy(&tex->tim_prect, &tparam.prect, sizeof(tparam.prect));
			tex->tpage = getTPage(tparam.mode & 0x3, 0, tparam.prect->x, tparam.prect->y);
		}
		LoadImage(tparam.prect, (u32*)tparam.paddr);
		DrawSync(0);
	}
	
	//Upload CLUT to framebuffer if present
	if ((tparam.mode & 0x8) && !(flag & GFX_LOADTEX_NOCLUT))
	{
		if (tex != NULL)
		{
			memcpy(&tex->tim_crect, &tparam.crect, sizeof(tparam.crect));
			tex->clut = getClut(tparam.crect->x, tparam.crect->y);
		}
		LoadImage(tparam.crect, (u32*)tparam.caddr);
		DrawSync(0);
	}
	
	//Free data
	if (flag & GFX_LOADTEX_FREE)
		Mem_Free(data);
}

void Gfx_BlitTex(Gfx_Tex *tex, const RECT *src, s32 x, s32 y)
{
	//Add sprite
	SPRT *sprt = (SPRT*)nextpri;
	setSprt(sprt);
	setXY0(sprt, x, y);
	setWH(sprt, src->w, src->h);
	setUV0(sprt, src->x, src->y);
	setRGB0(sprt, 128, 128, 128);
	sprt->clut = tex->clut;
	
	addPrim(ot[db], sprt);
	nextpri += sizeof(SPRT);
	
	//Add tpage change
	DR_TPAGE *tpage = (DR_TPAGE*)nextpri;
	setDrawTPage(tpage, 0, 1, tex->tpage);
	
	addPrim(ot[db], tpage);
	nextpri += sizeof(DR_TPAGE);
}

void Gfx_DrawTex(Gfx_Tex *tex, const RECT *src, const RECT *dst)
{
	//Add quad
	POLY_FT4 *quad = (POLY_FT4*)nextpri;
	setPolyFT4(quad);
	setUVWH(quad, src->x, src->y, src->w-1, src->h-1);
	setXYWH(quad, dst->x, dst->y, dst->w, dst->h);
	setRGB0(quad, 128, 128, 128);
	quad->tpage = tex->tpage;
	quad->clut = tex->clut;
	
	addPrim(ot[db], quad);
	nextpri += sizeof(POLY_FT4);
}
