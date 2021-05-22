#include "io.h"

#include "mem.h"
#include "audio.h"
#include "main.h"

//IO functions
void IO_Init()
{
	//Initialize CD IO
	CdInit();
}

IO_Data IO_Read(const char *path)
{
	printf("[IO_Read] Reading file %s\n", path);
	
	//Stop XA playback
	Audio_StopXA();
	
	//Search for file
	CdlFILE file;
	if (!CdSearchFile(&file, (char*)path))
	{
		sprintf(error_msg, "[IO_Read] %s not found", path);
		ErrorLock();
		return NULL;
	}
	size_t sects = (file.size + IO_SECT_SIZE - 1) / IO_SECT_SIZE;
	
	//Allocate a buffer for the file
	size_t size;
	IO_Data buffer = (IO_Data)Mem_Alloc(size = (IO_SECT_SIZE * sects));
	if (buffer == NULL)
	{
		sprintf(error_msg, "[IO_Read] Malloc (size %X) fail", size);
		ErrorLock();
		return NULL;
	}
	
	//Read file
	CdControl(CdlSetloc, (u8*)&file.pos, 0);
	CdRead(sects, buffer, CdlModeSpeed);
	CdReadSync(0, 0);
	
	return buffer;
}
