/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "menu/menu.h"

#include "main.h"

//Menu functions
void Menu_Load2(MenuPage page);

void Menu_Load(MenuPage page)
{
	//Load overlay then call load function
	Overlay_Load("\\MENU\\MENU.EXE;1");
	Menu_Load2(page);
	gameloop = GameLoop_Menu;
}
