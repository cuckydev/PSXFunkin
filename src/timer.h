/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _TIMER_H
#define _TIMER_H

#include "psx.h"
#include "fixed.h"

//Timer state
extern u32 frame_count, animf_count;
extern fixed_t timer_sec, timer_dt;

//Timer interface
void Timer_Init(void);
void Timer_Tick(void);
void Timer_Reset(void);

#endif
