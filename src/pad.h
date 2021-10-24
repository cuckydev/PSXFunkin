/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_PAD_H
#define PSXF_GUARD_PAD_H

#include "psx.h"

//Pad constants
#define PAD_SELECT      1
#define PAD_L3          2
#define PAD_R3          4
#define PAD_START       8
#define PAD_UP          16
#define PAD_RIGHT       32
#define PAD_DOWN        64
#define PAD_LEFT        128
#define PAD_L2          256
#define PAD_R2          512
#define PAD_L1          1024
#define PAD_R1          2048
#define PAD_TRIANGLE    4096
#define PAD_CIRCLE      8192
#define PAD_CROSS       16384
#define PAD_SQUARE      32768

//Pad structure
typedef struct
{
	u16 held, press;
	u8 left_x, left_y;
	u8 right_x, right_y;
} Pad;

extern Pad pad_state, pad_state_2;

#ifdef PSXF_NETWORK
	#define PAD_TYPE_CHARS 0x20
	extern char pad_type[PAD_TYPE_CHARS + 1];
	extern boolean pad_backspace;
#endif

//Pad functions
void Pad_Init(void);
void Pad_Quit(void);
void Pad_Update(void);

#endif
