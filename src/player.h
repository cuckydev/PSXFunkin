/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _PLAYER_H
#define _PLAYER_H

#include "character.h"

//Player enums
typedef enum
{
	PlayerAnim_Peace = CharAnim_Max,
	PlayerAnim_Sweat,
	
	PlayerAnim_Dead0, //BREAK
	PlayerAnim_Dead1, //Idle with mic
	PlayerAnim_Dead2, //Mic Drop
	PlayerAnim_Dead3, //Idle
	PlayerAnim_Dead4, //Body twitch
	PlayerAnim_Dead5, //Balls twitch
	
	PlayerAnim_Dead6, //Retry
	PlayerAnim_Dead7, //Blueball
	
	PlayerAnim_Max,
} PlayerAnim;

//Player structures
typedef struct
{
	s16 x, y;
	s16 xsp, ysp;
} SkullFragment;

#endif
