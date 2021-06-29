#ifndef _MENU_H
#define _MENU_H

//Menu enums
typedef enum
{
	MenuPage_Opening,
	MenuPage_Title,
	MenuPage_Main,
	MenuPage_Story,
	MenuPage_Freeplay,
	MenuPage_Mods,
	MenuPage_Options,
	
	MenuPage_Stage, //Changes game loop
} MenuPage;

//Menu functions
void Menu_Load(MenuPage page);
void Menu_Unload();
void Menu_Tick();

#endif
