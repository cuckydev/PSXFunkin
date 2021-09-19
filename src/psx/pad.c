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
Pad pad_state, pad_state_2;

//Internal pad functions
static void Pad_UpdateState(Pad *this, PADTYPE *pad)
{
	if (pad->stat == 0)
	{
		//Read pad information
		if ((pad->type == 0x4) ||
			(pad->type == 0x5) ||
			(pad->type == 0x7))
		{
			//Set pad state
			this->press = (~pad->btn) & (~this->held);
			this->held = ~pad->btn;
			this->left_x  = pad->ls_x;
			this->left_y  = pad->ls_y;
			this->right_x = pad->rs_x;
			this->right_y = pad->rs_y;
		}
	}
}

//Pad functions
void Pad_Init(void)
{
	
	//Clear pad states
	pad_state.held = pad_state.press = 0;
	pad_state.left_x = pad_state.left_y = pad_state.right_x = pad_state.right_y = 0;
	
	pad_state_2.held = pad_state_2.press = 0;
	pad_state_2.left_x = pad_state_2.left_y = pad_state_2.right_x = pad_state_2.right_y = 0;
	
	//Initialize system pads
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
	//Read pad states
	Pad_UpdateState(&pad_state,   (PADTYPE*)pad_buff[0]);
	Pad_UpdateState(&pad_state_2, (PADTYPE*)pad_buff[1]);
}
