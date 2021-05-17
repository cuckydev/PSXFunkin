#include "psx.h"

#include "io.h"
#include "gfx.h"
#include "audio.h"
#include "pad.h"

#include "menu.h"
#include "stage.h"

//Error handler
char error_msg[0x200];

void ErrorLock()
{
	while (1)
	{
		FntPrint("A fatal error has occured\n~c700%s\n", error_msg);
		Gfx_Flip();
	}
}

//Entry point
u_long malloc_heap[0x140000 / sizeof(u_long)];

int main()
{
	//Initialize system
	InitHeap3((void*)malloc_heap, sizeof(malloc_heap));
	
	Audio_Init();
	Gfx_Init();
	IO_Init();
	Pad_Init();
	
	//Start game
	int md = 0, stid = 0;
	Menu_Load(MenuLoadPage_Title);
	
	//Game loop
	while (1)
	{
		//Prepare frame
		Pad_Update();
		
		//Tick and draw game
		if (pad_state.press & PAD_START)
		{
			if (md)
				Stage_Unload();
			else
				Menu_Unload();
			Stage_Load(stid++, StageDiff_Hard);
			md = 1;
		}
		if (!md)
			Menu_Tick();
		else
			Stage_Tick();
		
		//Flip gfx buffers
		Gfx_Flip();
	}
	return 0;
}
