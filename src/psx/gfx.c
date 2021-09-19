/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../gfx.h"

#include "../mem.h"
#include "../main.h"

//Gfx constants
#define OTLEN 8

//Gfx state
DISPENV disp[2];
DRAWENV draw[2];
u8 db;

static u32 ot[2][OTLEN];    //Ordering table length
static u8 pribuff[2][32768]; //Primitive buffer
static u8 *nextpri;          //Next primitive pointer

//Gfx functions
void Gfx_Init(void)
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
	switch(*(char*)0xbfc7ff52)
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

void Gfx_Quit(void)
{
	
}

void Gfx_Flip(void)
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

void Gfx_EnableClear(void)
{
	draw[0].isbg = draw[1].isbg = 1;
}

void Gfx_DisableClear(void)
{
	draw[0].isbg = draw[1].isbg = 0;
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
			tex->tim_prect = *tparam.prect;
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
			tex->tim_crect = *tparam.crect;
			tex->clut = getClut(tparam.crect->x, tparam.crect->y);
		}
		LoadImage(tparam.crect, (u32*)tparam.caddr);
		DrawSync(0);
	}
	
	//Free data
	if (flag & GFX_LOADTEX_FREE)
		Mem_Free(data);
}

void Gfx_DrawRect(const RECT *rect, u8 r, u8 g, u8 b)
{
	//Add quad
	POLY_F4 *quad = (POLY_F4*)nextpri;
	setPolyF4(quad);
	setXYWH(quad, rect->x, rect->y, rect->w, rect->h);
	setRGB0(quad, r, g, b);
	
	addPrim(ot[db], quad);
	nextpri += sizeof(POLY_F4);
}

void Gfx_BlendRect(const RECT *rect, u8 r, u8 g, u8 b, u8 mode)
{
	//Add quad
	POLY_F4 *quad = (POLY_F4*)nextpri;
	setPolyF4(quad);
	setXYWH(quad, rect->x, rect->y, rect->w, rect->h);
	setRGB0(quad, r, g, b);
	setSemiTrans(quad, 1);
	
	addPrim(ot[db], quad);
	nextpri += sizeof(POLY_F4);
	
	//Add tpage change (this controls transparency mode)
	DR_TPAGE *tpage = (DR_TPAGE*)nextpri;
	setDrawTPage(tpage, 0, 1, getTPage(0, mode, 0, 0));
	
	addPrim(ot[db], tpage);
	nextpri += sizeof(DR_TPAGE);
}

void Gfx_BlitTexCol(Gfx_Tex *tex, const RECT *src, s32 x, s32 y, u8 r, u8 g, u8 b)
{
	//Add sprite
	SPRT *sprt = (SPRT*)nextpri;
	setSprt(sprt);
	setXY0(sprt, x, y);
	setWH(sprt, src->w, src->h);
	setUV0(sprt, src->x, src->y);
	setRGB0(sprt, r, g, b);
	sprt->clut = tex->clut;
	
	addPrim(ot[db], sprt);
	nextpri += sizeof(SPRT);
	
	//Add tpage change (TODO: reduce tpage changes)
	DR_TPAGE *tpage = (DR_TPAGE*)nextpri;
	setDrawTPage(tpage, 0, 1, tex->tpage);
	
	addPrim(ot[db], tpage);
	nextpri += sizeof(DR_TPAGE);
}

void Gfx_BlitTex(Gfx_Tex *tex, const RECT *src, s32 x, s32 y)
{
	Gfx_BlitTexCol(tex, src, x, y, 0x80, 0x80, 0x80);
}

void Gfx_DrawTexCol(Gfx_Tex *tex, const RECT *src, const RECT *dst, u8 r, u8 g, u8 b)
{
	//Manipulate rects to comply with GPU restrictions
	RECT csrc, cdst;
	csrc = *src;
	cdst = *dst;
	
	if (dst->w < 0)
		csrc.x--;
	if (dst->h < 0)
		csrc.y--;
	
	if ((csrc.x + csrc.w) >= 0x100)
	{
		csrc.w = 0xFF - csrc.x;
		cdst.w = cdst.w * csrc.w / src->w;
	}
	if ((csrc.y + csrc.h) >= 0x100)
	{
		csrc.h = 0xFF - csrc.y;
		cdst.h = cdst.h * csrc.h / src->h;
	}
	
	//Subdivide if particularly large
	if (csrc.w > 0x80)
	{
		RECT csrc2, cdst2;
		
		int srcs = csrc.w / 2;
		csrc2.x = csrc.x + srcs;
		csrc2.w = csrc.w - srcs;
		csrc2.y = csrc.y;
		csrc2.h = csrc.h;
		csrc.w = srcs;
		
		int dsts = cdst.w / 2;
		cdst2.x = cdst.x + dsts;
		cdst2.w = cdst.w - dsts;
		cdst2.y = cdst.y;
		cdst2.h = cdst.h;
		cdst.w = dsts;
		
		Gfx_DrawTexCol(tex, &csrc2, &cdst2, r, g, b);
	}
	if (csrc.h > 0x80)
	{
		RECT csrc2, cdst2;
		
		int srcs = csrc.h / 2;
		csrc2.x = csrc.x;
		csrc2.w = csrc.w;
		csrc2.y = csrc.y + srcs;
		csrc2.h = csrc.h - srcs;
		csrc.h = srcs;
		
		int dsts = cdst.h / 2;
		cdst2.x = cdst.x;
		cdst2.w = cdst.w;
		cdst2.y = cdst.y + dsts;
		cdst2.h = cdst.h - dsts;
		cdst.h = dsts;
		
		Gfx_DrawTexCol(tex, &csrc2, &cdst2, r, g, b);
	}
	
	//Add quad
	POLY_FT4 *quad = (POLY_FT4*)nextpri;
	setPolyFT4(quad);
	setUVWH(quad, csrc.x, csrc.y, csrc.w, csrc.h);
	setXYWH(quad, cdst.x, cdst.y, cdst.w, cdst.h);
	setRGB0(quad, r, g, b);
	quad->tpage = tex->tpage;
	quad->clut = tex->clut;
	
	addPrim(ot[db], quad);
	nextpri += sizeof(POLY_FT4);
}

void Gfx_DrawTex(Gfx_Tex *tex, const RECT *src, const RECT *dst)
{
	Gfx_DrawTexCol(tex, src, dst, 0x80, 0x80, 0x80);
}

void Gfx_DrawTexArbCol(Gfx_Tex *tex, const RECT *src, const POINT *p0, const POINT *p1, const POINT *p2, const POINT *p3, u8 r, u8 g, u8 b)
{
	//Add quad
	POLY_FT4 *quad = (POLY_FT4*)nextpri;
	setPolyFT4(quad);
	setUVWH(quad, src->x, src->y, src->w, src->h);
	setXY4(quad, p0->x, p0->y, p1->x, p1->y, p2->x, p2->y, p3->x, p3->y);
	setRGB0(quad, r, g, b);
	quad->tpage = tex->tpage;
	quad->clut = tex->clut;
	
	addPrim(ot[db], quad);
	nextpri += sizeof(POLY_FT4);
}

void Gfx_DrawTexArb(Gfx_Tex *tex, const RECT *src, const POINT *p0, const POINT *p1, const POINT *p2, const POINT *p3)
{
	Gfx_DrawTexArbCol(tex, src, p0, p1, p2, p3, 0x80, 0x80, 0x80);
}
