#include "../gfx.h"

#include "../main.h"

#include "SDL_render.h"
#include "SDL_events.h"

//Window and renderer
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

#define WINDOW_SCALE 3

//Clear state
u8 clear_r, clear_g, clear_b, clear_e;

//Gfx functions
void Gfx_Init(void)
{
	//Create window
	if ((window = SDL_CreateWindow("PSXFunkin", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * WINDOW_SCALE, SCREEN_HEIGHT * WINDOW_SCALE, 0)) == NULL)
	{
		sprintf(error_msg, "[Gfx_Init] Failed to create SDL2 window: %s", SDL_GetError());
		ErrorLock();
	}
	
	//Create renderer
	if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC)) == NULL)
	{
		sprintf(error_msg, "[Gfx_Init] Failed to create SDL2 renderer: %s", SDL_GetError());
		ErrorLock();
	}
	
	//Initialize render state
	clear_r = 0;
	clear_g = 0;
	clear_b = 0;
	clear_e = 1;
	
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void Gfx_Flip(void)
{
	//Present renderer
	SDL_RenderPresent(renderer);
	
	//Pump events
	SDL_Event event;
    while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				exit(0);
				break;
			default:
				break;
		}
	}
	
	//Clear screen
	if (clear_e)
	{
		SDL_SetRenderDrawColor(renderer, clear_r, clear_g, clear_b, 0xFF);
		SDL_RenderClear(renderer);
	}
}

void Gfx_SetClear(u8 r, u8 g, u8 b)
{
	clear_r = r;
	clear_g = g;
	clear_b = b;
}

void Gfx_EnableClear(void)
{
	clear_e = 1;
}

void Gfx_DisableClear(void)
{
	clear_e = 0;
}

void Gfx_LoadTex(Gfx_Tex *tex, IO_Data data, Gfx_LoadTex_Flag free)
{
	
}

void Gfx_DrawRect(const RECT *rect, u8 r, u8 g, u8 b)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, 0xFF);
	SDL_RenderFillRect(renderer, (const SDL_Rect*)rect);
}

void Gfx_BlitTexCol(Gfx_Tex *tex, const RECT *src, s32 x, s32 y, u8 r, u8 g, u8 b)
{
	if (r & 0x80)
		r = 0xFF;
	else
		r <<= 1;
	if (g & 0x80)
		g = 0xFF;
	else
		g <<= 1;
	if (b & 0x80)
		b = 0xFF;
	else
		b <<= 1;
	RECT dst;
	dst.x = x;
	dst.y = y;
	dst.w = src->w;
	dst.h = src->h;
	Gfx_DrawRect(&dst, r, g, b);
}

void Gfx_BlitTex(Gfx_Tex *tex, const RECT *src, s32 x, s32 y)
{
	Gfx_BlitTexCol(tex, src, x, y, 0x7F, 0x7F, 0x7F);
}

void Gfx_DrawTexCol(Gfx_Tex *tex, const RECT *src, const RECT *dst, u8 r, u8 g, u8 b)
{
	if (r & 0x80)
		r = 0xFF;
	else
		r <<= 1;
	if (g & 0x80)
		g = 0xFF;
	else
		g <<= 1;
	if (b & 0x80)
		b = 0xFF;
	else
		b <<= 1;
	Gfx_DrawRect(dst, r, g, b);
}

void Gfx_DrawTex(Gfx_Tex *tex, const RECT *src, const RECT *dst)
{
	Gfx_DrawTexCol(tex, src, dst, 0x7F, 0x7F, 0x7F);
}

void Gfx_DrawTexArb(Gfx_Tex *tex, const RECT *src, const POINT *p0, const POINT *p1, const POINT *p2, const POINT *p3)
{
	
}
