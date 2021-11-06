/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "main.h"

#include "timer.h"
#include "io.h"
#include "gfx.h"
#include "audio.h"
#include "pad.h"
#include "network.h"

#include "menu/menu.h"
#include "stage.h"

//Memory implementation
#define MEM_STAT //This will enable the Mem_GetStat function which returns information about available memory in the heap

#define MEM_IMPLEMENTATION
#include "mem.h"
#undef MEM_IMPLEMENTATION

//Game loop
GameLoop gameloop;

//Error handler
char error_msg[0x200];

void ErrorLock(void)
{
	while (1)
	{
		#ifdef PSXF_PC
			MsgPrint(error_msg);
			exit(1);
		#else
			FntPrint("A fatal error has occured\n~c700%s\n", error_msg);
			Gfx_Flip();
		#endif
	}
}

//Overlay interface
#ifndef PSXF_PC

extern u8 __heap_start, __ram_top;

static int overlay_pos;
static u16 *overlay_sizes;
static IO_Data overlay_data;

void Overlay_Load(const char *path)
{
	//Find file
	CdlFILE file;
	IO_FindFile(&file, path);
	
	//Read first overlay sector
	overlay_pos = CdPosToInt(&file.pos);
	
	CdIntToPos(overlay_pos, &file.pos);
	CdControl(CdlSetloc, (u8*)&file.pos, NULL);
	
	CdRead(1, (IO_Data)&__heap_start, CdlModeSpeed);
	CdReadSync(0, NULL);
	
	overlay_pos += 1;
	
	//Read the rest of the overlay
	size_t overlay_sectsleft = *((u16*)&__heap_start);
	size_t overlay_size = (overlay_sectsleft + 1) << 11;
	
	CdIntToPos(overlay_pos, &file.pos);
	CdControl(CdlSetloc, (u8*)&file.pos, NULL);
	
	CdRead(overlay_sectsleft, (IO_Data)(&__heap_start + 0x800), CdlModeSpeed);
	CdReadSync(0, NULL);
	
	overlay_pos += overlay_sectsleft;
	
	//Initialize memory heap at end of overlay data
	Mem_Init(&__heap_start + overlay_size, &__ram_top - &__heap_start - overlay_size);
	
	//Allocate overlay data buffers
	overlay_sizes = (u16*)(&__heap_start + (overlay_sectsleft << 11));
	
	u16 overlay_sizemax = 0;
	for (u16 *overlay_sizep = overlay_sizes; *overlay_sizep != 0; overlay_sizep++)
		if (*overlay_sizep > overlay_sizemax)
			overlay_sizemax = *overlay_sizep;
	
	if ((overlay_data = Mem_Alloc(overlay_sizemax << 11)) == NULL)
	{
		sprintf(error_msg, "[Overlay_Load] Failed to allocate overlay data buffer (%d sectors)", overlay_sizemax);
		ErrorLock();
	}
}

IO_Data Overlay_DataRead(void)
{
	printf("Reading %d overlay sectors\n", *overlay_sizes);
	
	//Read data to overlay data buffer according to sizes
	CdlLOC pos;
	
	CdIntToPos(overlay_pos, &pos);
	CdControl(CdlSetloc, (u8*)&pos, NULL);
	
	CdRead(*overlay_sizes, overlay_data, CdlModeSpeed);
	CdReadSync(0, NULL);
	
	overlay_pos += *overlay_sizes++;
	return overlay_data;
}

void Overlay_DataFree(void)
{
	//Free overlay data buffer
	Mem_Free(overlay_data);
}

#endif

//Entry point
int main(int argc, char **argv)
{
	//Remember arguments
	my_argc = argc;
	my_argv = argv;
	
	//Initialize system
	PSX_Init();
	
	IO_Init();
	Audio_Init();
	Gfx_Init();
	Pad_Init();
	Network_Init();
	
	Timer_Init();
	
	//Start game
	Menu_Load(MenuPage_Opening);
	
	//Game loop
	while (PSX_Running())
	{
		//Prepare frame
		Timer_Tick();
		Audio_ProcessXA();
		Pad_Update();
		
		#ifdef MEM_STAT
			//Memory stats
			size_t mem_used, mem_size, mem_max;
			Mem_GetStat(&mem_used, &mem_size, &mem_max);
			#ifndef MEM_BAR
				FntPrint("mem: %08X/%08X (max %08X)\n", mem_used, mem_size, mem_max);
			#endif
		#endif
		
		//Tick and draw game
		Network_Process();
		switch (gameloop)
		{
			case GameLoop_Menu:
				Menu_Tick();
				break;
			case GameLoop_Stage:
				Stage_Tick();
				break;
		}
		
		//Flip gfx buffers
		Gfx_Flip();
	}
	
	//Deinitialize system
	Network_Quit();
	Pad_Quit();
	Gfx_Quit();
	Audio_Quit();
	IO_Quit();
	
	PSX_Quit();
	return 0;
}
