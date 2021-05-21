#include "menu.h"
#include "io.h"
#include "gfx.h"
#include "audio.h"
#include "pad.h"


#include "stage.h"
extern int stid;

//Menu state
typedef enum
{
	MenuPage_MsgCredits,    //Creator credit
	MenuPage_MsgNewgrounds, //Newgrounds credit
	MenuPage_MsgPortCredit, //Port credit
	MenuPage_MsgFunkin,     //Friday Night Funkin' text
	MenuPage_Title,         //Title screen
} MenuPage;

typedef struct
{
	//Menu state
	MenuPage page;
	
	s32 frame;
	
	Gfx_Tex tex_whitefnt;
	
	//Title state
	IO_Data data_title0, data_title1, data_gf[5];
	Gfx_Tex tex_title, tex_ng;
} Menu;

static Menu menu;

//Menu drawing
#define TEXT_W 13

void Menu_DrawText(const char *text, s32 x, s32 y)
{
	u32 v0 = 0;
	u8 v1 = (menu.frame >> 3) & 1;
	
	u8 c;
	while ((c = *text++) != '\0')
	{
		//Draw character
		if ((c -= 'A') <= 'Z'-'A')
		{
			RECT src = {((c & 0x7) << 5) + ((((v0 >> c) & 1) ^ v1) << 4), (c & ~0x7) << 1, 16, 16};
			Gfx_BlitTex(&menu.tex_whitefnt, &src, x, y);
			v0 ^= 1 << c;
		}
		x += 13;
	}
}

void Menu_DrawCenterText(const char *text, s32 x, s32 y)
{
	Menu_DrawText(text, x - (strlen(text) * TEXT_W / 2), y);
}

//Menu functions
void Menu_Load(MenuLoadPage page)
{
	//Load menu textures
	Gfx_LoadTex(&menu.tex_whitefnt, IO_Read("\\MENU\\WHITEFNT.TIM;1"), GFX_LOADTEX_FREE);
	
	//Initialize page
	menu.data_title0 = NULL;
	menu.data_title1 = NULL;
	
	switch (page)
	{
		case MenuLoadPage_Title:
			//Load title textures
			Gfx_LoadTex(&menu.tex_ng, IO_Read("\\MENU\\NG.TIM;1"), GFX_LOADTEX_FREE);
			menu.data_title0 = IO_Read("\\MENU\\TITLE0.TIM;1");
			menu.data_title1 = IO_Read("\\MENU\\TITLE1.TIM;1");
			menu.data_gf[0] = IO_Read("\\MENU\\GF0.TIM;1");
			menu.data_gf[1] = IO_Read("\\MENU\\GF1.TIM;1");
			menu.data_gf[2] = IO_Read("\\MENU\\GF2.TIM;1");
			menu.data_gf[3] = IO_Read("\\MENU\\GF3.TIM;1");
			menu.data_gf[4] = IO_Read("\\MENU\\GF4.TIM;1");
			
			//Set page
			menu.page = MenuPage_MsgCredits;
			break;
		default:
			break;
	}
	
	//Initialize menu state
	menu.frame = 0;
	
	//Play menu music
	Audio_PlayXA("\\MUSIC\\MENU.XA;1", 0x40, 0, 1);
}

void Menu_Unload()
{
	//Free buffers
	free3(menu.data_title0);
	free3(menu.data_title1);
	free3(menu.data_gf[0]);
	free3(menu.data_gf[1]);
	free3(menu.data_gf[2]);
	free3(menu.data_gf[3]);
	free3(menu.data_gf[4]);
}

void Menu_Tick()
{
	//Debug
	static const char *titles[] = {
		"BOPEEBO",
		"FRESH",
		"DADBATTLE",
		
		"PICO",
		"PHILY",
		"BLAMMED",
		
		"SATIN PANTIES",
		"HIGH",
		"MILF",
		
		"TEST",
	};
	
	if (pad_state.press & PAD_DOWN)
		if (++stid > StageId_4_4)
			stid = StageId_4_4;
	if (pad_state.press & PAD_UP)
		if (--stid < 0)
			stid = 0;
	Menu_DrawText("STAGE SELECT", 16, 16);
	Menu_DrawText(titles[stid], 16, 32);
	
	//Run current menu page
	switch (menu.page)
	{
		case MenuPage_MsgCredits:
		{
			//Draw credits
			if (menu.frame >= 30)
			{
				Menu_DrawCenterText("NINJAMUFFIN",   SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32);
				Menu_DrawCenterText("PHANTOMARCADE", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 16);
				Menu_DrawCenterText("KAWAISPRITE",   SCREEN_WIDTH2, SCREEN_HEIGHT2);
				Menu_DrawCenterText("EVILSKER",      SCREEN_WIDTH2, SCREEN_HEIGHT2 + 16);
			}
			
			//Handle page switch
			if (++menu.frame >= 165)
			{
				menu.page = MenuPage_MsgNewgrounds;
				menu.frame = -15;
			}
			break;
		}
		case MenuPage_MsgNewgrounds:
		{
			//Draw credits
			if (menu.frame >= 0)
			{
				Menu_DrawCenterText("IN ASSOCIATION", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64);
				Menu_DrawCenterText("WITH",           SCREEN_WIDTH2, SCREEN_HEIGHT2 - 48);
			}
			if (menu.frame >= 70)
			{
				Menu_DrawCenterText("NEWGROUNDS",     SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32);
				RECT src = {0, 0, 128, 128};
				Gfx_BlitTex(&menu.tex_ng, &src, SCREEN_WIDTH2 - 64, SCREEN_HEIGHT2 - 16);
			}
			
			//Handle page switch
			if (++menu.frame >= 115)
			{
				menu.page = MenuPage_MsgPortCredit;
				menu.frame = -15;
			}
			break;
		}
		case MenuPage_MsgPortCredit: //This would display a random message in the original, nothing to seed from though
		{
			//Draw text
			if (menu.frame >= 0)
			{
				Menu_DrawCenterText("PSX PORT BY", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32);
				Menu_DrawCenterText("CUCKYDEV",    SCREEN_WIDTH2, SCREEN_HEIGHT2 - 16);
			}
			if (menu.frame >= 75)
			{
				Menu_DrawCenterText("I LIED THIS IS", SCREEN_WIDTH2, SCREEN_HEIGHT2 + 16);
				Menu_DrawCenterText("UNOFFICIAL",     SCREEN_WIDTH2, SCREEN_HEIGHT2 + 32);
			}
			
			//Handle page switch
			if (++menu.frame >= 115)
			{
				menu.page = MenuPage_MsgFunkin;
				menu.frame = -15;
			}
			break;
		}
		case MenuPage_MsgFunkin:
		{
			//Draw text
			if (menu.frame >= 0)
				Menu_DrawCenterText("FRIDAY", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 48);
			if (menu.frame >= 25)
				Menu_DrawCenterText("NIGHT",  SCREEN_WIDTH2, SCREEN_HEIGHT2 - 8);
			if (menu.frame >= 50)
				Menu_DrawCenterText("FUNKIN", SCREEN_WIDTH2, SCREEN_HEIGHT2 + 32);
			
			//Handle page switch
			if (++menu.frame >= 95)
			{
				menu.page = MenuPage_Title;
				menu.frame = -15;
			}
			break;
		}
		case MenuPage_Title:
		{
			if (menu.frame >= 0)
			{
				//Draw press start prompt
				Menu_DrawCenterText("PRESS START", SCREEN_WIDTH2, SCREEN_HEIGHT - 32);
				
				//Draw title
				const u32 beattime = 35;
				
				s32 title_cnt = menu.frame % beattime;
				s32 title_y, title_xo, title_yo;
				if (title_cnt == 0)
					Gfx_LoadTex(&menu.tex_title, menu.data_title0, 0);
				else if (title_cnt == beattime - 4)
					Gfx_LoadTex(&menu.tex_title, menu.data_title1, 0);
				
				if (title_cnt < beattime - 6)
				{ title_y = 0; title_xo = 171 / 2; title_yo = 107 / 2; }
				else if (title_cnt < beattime - 4)
				{ title_y = 128; title_xo = 180 / 2; title_yo = 112 / 2; }
				else if (title_cnt < beattime - 2)
				{ title_y = 0; title_xo = 175 / 2; title_yo = 108 / 2; }
				else
				{ title_y = 128; title_xo = 173 / 2; title_yo = 108 / 2; }
				
				RECT title_src = {0, title_y, 196, 128};
				RECT psx_src = {196, 0, 64, 16};
				Gfx_BlitTex(&menu.tex_title, &title_src, 100 - title_xo, 78 - title_yo);
				Gfx_BlitTex(&menu.tex_title, &psx_src, 100 - 32, 78 + title_yo);
				
				//Draw girlfriend
				if ((menu.frame & 0x7) == 0)
					Gfx_LoadTex(&menu.tex_ng, menu.data_gf[(menu.frame >> 3) % 5], 0);
				
				u8 gf_sf = menu.frame >> 2;
				RECT gf_src = {(gf_sf & 1) << 7, (gf_sf & 2) << 6, 128, 128};
				RECT gf_dst = {SCREEN_WIDTH2 - 16, SCREEN_HEIGHT2 - 80, 168, 168};
				Gfx_DrawTex(&menu.tex_ng, &gf_src, &gf_dst);
			}
			
			//Increment frame count
			menu.frame++;
			break;
		}
	}
}
