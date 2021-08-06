#ifndef _TIMER_H
#define _TIMER_H

#include "fixed.h"

//Timer state
extern u32 frame_count, animf_count;
extern fixed_t timer_sec, timer_dt;

//Timer interface
void Timer_Init(void);
void Timer_Tick(void);

#endif