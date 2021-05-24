#include "psx.h"

#include "mem.h"
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
u32 malloc_heap[0x100000 / sizeof(u32)];

int stid;

int main()
{
	//Initialize system
	Mem_Init((void*)malloc_heap, sizeof(malloc_heap));
	
	Audio_Init();
	Gfx_Init();
	IO_Init();
	Pad_Init();
	
	//Start game
	boolean md = 0;
	stid = StageId_1_1;
	Menu_Load(MenuLoadPage_Title);
	
	//Game loop
	while (1)
	{
		//Prepare frame
		Audio_ProcessXA();
		Pad_Update();
		
		//Tick and draw game
		if (pad_state.press & PAD_START)
		{
			if (md)
			{
				Stage_Unload();
				Menu_Load(MenuLoadPage_Title);
			}
			else
			{
				Menu_Unload();
				Stage_Load(stid, StageDiff_Hard);
			}
			md ^= 1;
		}
		if (!md)
			Menu_Tick();
		else
			Stage_Tick();
		
		#ifdef MEM_STAT
			//Memory stats
			size_t mem_used, mem_size, mem_max;
			Mem_GetStat(&mem_used, &mem_size, &mem_max);
			FntPrint("mem: %08X/%08X (max %08X)\n", mem_used, mem_size, mem_max);
		#endif
		
		//Flip gfx buffers
		Gfx_Flip();
	}
	return 0;
}
