#include "player.h"

//Players
#include "player/bf.h"

static Player* (*player_new[PlayerId_Max])(fixed_t, fixed_t) = {
	Player_BF_New,
};

//Player functions
Player *Player_New(PlayerId id, fixed_t x, fixed_t y)
{
	//Allocate new player from given id
	return player_new[id](x, y);
}
