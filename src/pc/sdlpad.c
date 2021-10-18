/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../pad.h"

#include "SDL.h"

//Pad state
Pad pad_state, pad_state_2;

#ifdef PSXF_NETWORK

//Stupid type code
char pad_type[PAD_TYPE_CHARS + 1];
boolean pad_backspace, pad_backspace_held;

#endif

//Pad functions
void Pad_Init(void)
{
	//Clear pad states
	pad_state.held = pad_state.press = 0;
	pad_state.left_x = pad_state.left_y = pad_state.right_x = pad_state.right_y = 0;
	
	pad_state_2.held = pad_state_2.press = 0;
	pad_state_2.left_x = pad_state_2.left_y = pad_state_2.right_x = pad_state_2.right_y = 0;
}

void Pad_Quit(void)
{
	
}

void Pad_Update(void)
{
	u16 next_held;
	
	//Get held keys
	Uint8 *keystate = SDL_GetKeyboardState(NULL);
	
	//Get next held state
	next_held = 0;
	if (keystate[SDL_SCANCODE_D])
		next_held |= PAD_SQUARE;
	if (keystate[SDL_SCANCODE_F])
		next_held |= PAD_CROSS;
	if (keystate[SDL_SCANCODE_J])
		next_held |= PAD_TRIANGLE;
	if (keystate[SDL_SCANCODE_K])
		next_held |= PAD_CIRCLE;
	if (keystate[SDL_SCANCODE_LEFT])
		next_held |= PAD_LEFT;
	if (keystate[SDL_SCANCODE_UP])
		next_held |= PAD_UP;
	if (keystate[SDL_SCANCODE_RIGHT])
		next_held |= PAD_RIGHT;
	if (keystate[SDL_SCANCODE_DOWN])
		next_held |= PAD_DOWN;
	if (keystate[SDL_SCANCODE_RETURN])
		next_held |= PAD_START;
	
	//Update pad state
	pad_state.press = next_held & ~pad_state.held;
	pad_state.held = next_held;
	
	//Get next held state 2
	next_held = 0;
	if (keystate[SDL_SCANCODE_E])
		next_held |= PAD_SQUARE;
	if (keystate[SDL_SCANCODE_R])
		next_held |= PAD_CROSS;
	if (keystate[SDL_SCANCODE_U])
		next_held |= PAD_TRIANGLE;
	if (keystate[SDL_SCANCODE_I])
		next_held |= PAD_CIRCLE;
	
	//Update pad state 2
	pad_state_2.press = next_held & ~pad_state_2.held;
	pad_state_2.held = next_held;
}
