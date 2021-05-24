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
	IO_Data buffer = IO_AsyncRead(path);
	CdReadSync(0, NULL); //Wait for reading to complete
	return buffer;
}

IO_Data IO_AsyncRead(const char *path)
{
	printf("[IO_ReadAsync] Reading file %s\n", path);
	
	//Stop XA playback
	Audio_StopXA();
	
	//Search for file
	CdlFILE file;
	if (!CdSearchFile(&file, (char*)path))
	{
		sprintf(error_msg, "[IO_ReadAsync] %s not found", path);
		ErrorLock();
		return NULL;
	}
	size_t sects = (file.size + IO_SECT_SIZE - 1) / IO_SECT_SIZE;
	
	//Allocate a buffer for the file
	size_t size;
	IO_Data buffer = (IO_Data)Mem_Alloc(size = (IO_SECT_SIZE * sects));
	if (buffer == NULL)
	{
		sprintf(error_msg, "[IO_ReadAsync] Malloc (size %X) fail", size);
		ErrorLock();
		return NULL;
	}
	
	//Read file
	CdControl(CdlSetloc, (u8*)&file.pos, NULL);
	CdRead(sects, buffer, CdlModeSpeed);
	
	return buffer; //NOT FILLED IN UNTIL IO_IsReading RETURNS FALSE
}

boolean IO_IsSeeking()
{
	CdControl(CdlNop, NULL, NULL);
	return (CdStatus() & (CdlStatSeek)) != 0;
}

boolean IO_IsReading()
{
	CdControl(CdlNop, NULL, NULL);
	return (CdStatus() & (CdlStatSeek | CdlStatRead)) != 0;
}
