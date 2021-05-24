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
	
	PlayerAnim_Max,
} PlayerAnim;

//Player structure
typedef struct Player
{
	//Character base structure
	Character character;
} Player;

//Player functions
Player *Player_New(PlayerId id, fixed_t x, fixed_t y);

#endif
