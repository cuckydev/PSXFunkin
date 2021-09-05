/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../pad.h"

//Pad state
typedef struct
{
    u8  stat;
    u8  len : 4;
    u8  type : 4;
    u16 btn;
    u8  rs_x, rs_y;
    u8  ls_x, ls_y;
} PADTYPE;

static u16 pad_buff[2][34/2];
Pad pad_state;

//Pad functions
void Pad_Init(void)
{
	//Initialize and start pad interface
	pad_state.held = 0;
	pad_state.press = 0;
	
	InitPAD((char*)pad_buff[0], 34, (char*)pad_buff[1], 34);
	pad_buff[0][0] = 0xFFFF;
	pad_buff[1][0] = 0xFFFF;
	StartPAD();
	
	ChangeClearPAD(0);
}

void Pad_Quit(void)
{
	
}

void Pad_Update(void)
{
	//Read port 1 state
	PADTYPE *pad = (PADTYPE*)pad_buff[0];
	
	if (pad->stat == 0)
	{
		//Read pad information
		if ((pad->type == 0x4) ||
			(pad->type == 0x5) ||
			(pad->type == 0x7))
		{
			//Set pad state
			pad_state.press = (~pad->btn) & (~pad_state.held);
			pad_state.held = ~pad->btn;
			pad_state.left_x  = pad->ls_x;
			pad_state.left_y  = pad->ls_y;
			pad_state.right_x = pad->rs_x;
			pad_state.right_y = pad->rs_y;
		}
	}
}
