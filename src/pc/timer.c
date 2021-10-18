/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../timer.h"

#ifdef PSXF_EMSCRIPTEN
	#include "SDL.h"
#else
	#define GLFW_INCLUDE_NONE
	#include <GLFW/glfw3.h>
#endif

//Timer state
u32 frame_count, animf_count;
fixed_t timer_sec, timer_dt;

//Timer interface
void Timer_Init(void)
{
	Timer_Reset();
}

void Timer_Tick(void)
{
	//Increment frame count
	frame_count++;
	
	//Update seconds counter
	#ifdef PSXF_EMSCRIPTEN
		fixed_t nsec = (fixed_t)(SDL_GetTicks() << FIXED_SHIFT) / 1000;
	#else
		fixed_t nsec = (fixed_t)(glfwGetTime() * FIXED_UNIT);
	#endif
	timer_dt = nsec - timer_sec;
	timer_sec = nsec;
	
	//Update 24fps counter
	animf_count = (timer_sec * 24) >> FIXED_SHIFT;
}

void Timer_Reset(void)
{
	//Update seconds counter
	#ifdef PSXF_EMSCRIPTEN
		fixed_t nsec = (fixed_t)(SDL_GetTicks() << FIXED_SHIFT) / 1000;
	#else
		fixed_t nsec = (fixed_t)(glfwGetTime() * FIXED_UNIT);
	#endif
	timer_sec = nsec;
	timer_dt = 0;
}
