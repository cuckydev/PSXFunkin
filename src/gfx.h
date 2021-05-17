#ifndef _GFX_H
#define _GFX_H

#include "psx.h"
#include "io.h"

//Gfx constants
#define SCREEN_WIDTH   320
#define SCREEN_HEIGHT  240
#define SCREEN_WIDTH2  (SCREEN_WIDTH >> 1)
#define SCREEN_HEIGHT2 (SCREEN_HEIGHT >> 1)

//Gfx structures
typedef struct
{
	int tim_mode;
	RECT tim_prect, tim_crect;
	u_short tpage, clut;
	u_char pxshift;
} Gfx_Tex;

//Gfx functions
void Gfx_Init();
void Gfx_Flip();

typedef u_char Gfx_LoadTex_Flag;
#define GFX_LOADTEX_FREE   (1 << 0)
#define GFX_LOADTEX_NOTEX  (1 << 1)
#define GFX_LOADTEX_NOCLUT (1 << 2)
void Gfx_LoadTex(Gfx_Tex *tex, IO_Data data, Gfx_LoadTex_Flag free);

void Gfx_BlitTex(Gfx_Tex *tex, const RECT *src, int x, int y);
void Gfx_DrawTex(Gfx_Tex *tex, const RECT *src, const RECT *dst);

#endif
