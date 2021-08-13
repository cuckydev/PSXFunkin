#include "trans.h"

#include "gfx.h"

#define TRANS_COVER_INC 8

//Transition state
typedef enum
{
	TransState_Idle,
	TransState_Out,
	TransState_In,
} TransState;

static struct
{
	//Transition state
	TransState state;
	u16 cover;
} transition;

//Transition functions
void Trans_Set(void)
{
	//Initialize transition fading in
	transition.state = TransState_In;
	transition.cover = (SCREEN_HEIGHT + TRANS_COVER_INC - 1) / TRANS_COVER_INC * TRANS_COVER_INC;
}

void Trans_Clear(void)
{
	//Initialize transition idle
	transition.state = TransState_Idle;
	transition.cover = 0;
}

void Trans_Start(void)
{
	//Start fading out transition
	transition.state = TransState_Out;
}

boolean Trans_Tick(void)
{
	//Handle transition by state
	switch (transition.state)
	{
		default:
			return false;
		case TransState_In:
		{
			//Decrement transition coverage
			if ((transition.cover -= TRANS_COVER_INC) == 0)
			{
				transition.state = TransState_Idle;
				return false;
			}
			
			//Draw transition cover
			RECT trans_rect = {
				0,
				SCREEN_HEIGHT - transition.cover,
				SCREEN_WIDTH,
				transition.cover
			};
			Gfx_DrawRect(&trans_rect, 0, 0, 0);
			
			return false;
		}
		case TransState_Out:
		{
			//Increment transition coverage
			boolean result;
			if (transition.cover >= SCREEN_HEIGHT)
			{
				transition.state = TransState_In;
				result = true;
			}
			else
			{
				result = false;
			}
			transition.cover += TRANS_COVER_INC;
			
			//Draw transition cover
			RECT trans_rect = {
				0,
				0,
				SCREEN_WIDTH,
				transition.cover
			};
			Gfx_DrawRect(&trans_rect, 0, 0, 0);
			return result;
		}
	}
}

boolean Trans_Idle(void)
{
	return transition.state == TransState_Idle;
}
