/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_MAIN_H
#define PSXF_GUARD_MAIN_H

//Game loop
typedef enum
{
	GameLoop_Menu,
	GameLoop_Stage,
} GameLoop;

extern GameLoop gameloop;

//Error handler
extern char error_msg[0x200];
void ErrorLock();

#endif
