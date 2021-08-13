#include "../timer.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

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
	fixed_t nsec = (fixed_t)(glfwGetTime() * FIXED_UNIT);
	timer_dt = nsec - timer_sec;
	timer_sec = nsec;
	
	//Update 24fps counter
	animf_count = (timer_sec * 24) >> FIXED_SHIFT;
}

void Timer_Reset(void)
{
	//Update seconds counter
	fixed_t nsec = (fixed_t)(glfwGetTime() * FIXED_UNIT);
	timer_sec = nsec;
	timer_dt = 0;
}
