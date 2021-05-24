#ifndef _PLAYER_H
#define _PLAYER_H

#include "character.h"

//Player enums
typedef enum
{
	PlayerId_BF,
	
	PlayerId_Max,
} PlayerId;

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

typedef struct Player
{
	//Character base structure
	Character character;
} Player;

//Player functions
Player *Player_New(PlayerId id, fixed_t x, fixed_t y);

#endif
