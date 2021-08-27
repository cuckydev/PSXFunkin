#include "trans.h"

#include "gfx.h"

#define TRANS_COVER_INC 10
#define TRANS_FADE_LEN 64

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
	s16 cover;
} transition;

//Transition functions
void Trans_Set(void)
{
	//Initialize transition fading in
	transition.state = TransState_In;
	transition.cover = SCREEN_HEIGHT;
}

void Trans_Clear(void)
{
	//Initialize transition idle
	transition.state = TransState_Idle;
	transition.cover = -TRANS_FADE_LEN;
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
			if ((transition.cover -= TRANS_COVER_INC) <= -TRANS_FADE_LEN)
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
			
			RECT trans_fade = {
				0,
				SCREEN_HEIGHT - transition.cover - TRANS_FADE_LEN,
				SCREEN_WIDTH,
				1
			};
			for (int i = 0; i < TRANS_FADE_LEN; i++)
			{
				Gfx_BlendRect(&trans_fade, 255 * i / TRANS_FADE_LEN, 255 * i / TRANS_FADE_LEN, 255 * i / TRANS_FADE_LEN, 2);
				trans_fade.y++;
			}
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
			
			RECT trans_fade = {
				0,
				transition.cover + TRANS_FADE_LEN,
				SCREEN_WIDTH,
				1
			};
			for (int i = 0; i < TRANS_FADE_LEN; i++)
			{
				trans_fade.y--;
				Gfx_BlendRect(&trans_fade, 255 * i / TRANS_FADE_LEN, 255 * i / TRANS_FADE_LEN, 255 * i / TRANS_FADE_LEN, 2);
			}
			return result;
		}
	}
}

boolean Trans_Idle(void)
{
	return transition.state == TransState_Idle;
}
