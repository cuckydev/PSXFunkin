#ifndef _MENU_H
#define _MENU_H

//Menu enums
typedef enum
{
	MenuLoadPage_Title,
	MenuLoadPage_Main,
	MenuLoadPage_Story,
	MenuLoadPage_Freeplay,
} MenuLoadPage;

//Menu functions
void Menu_Load(MenuLoadPage page);
void Menu_Unload();
void Menu_Tick();

#endif
