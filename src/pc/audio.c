#include "../audio.h"

#include "../io.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

//XA state
static IO_Data xa_data;
static size_t xa_sectors;

static double xa_time;

//XA files and tracks
#include "../audio_def.h"

//Audio functions
void Audio_Init(void)
{
	
	//Get file positions
	CdlFILE *filep = xa_files;
	for (const char **pathp = xa_paths; *pathp != NULL; pathp++)
		IO_FindFile(filep++, *pathp);
}

void Audio_GetXAFile(CdlFILE *file, XA_Track track)
{
	
}

void Audio_PlayXA_File(CdlFILE *file, u8 volume, u8 channel, boolean loop)
{
	xa_time = glfwGetTime();
}

void Audio_PlayXA_Track(XA_Track track, u8 volume, u8 channel, boolean loop)
{
	xa_time = glfwGetTime();
}

void Audio_PlayXA(const char *path, u8 volume, u8 channel, boolean loop)
{
	xa_time = glfwGetTime();
}

void Audio_PauseXA(void)
{
	
}

void Audio_StopXA(void)
{
	
}

void Audio_ChannelXA(u8 channel)
{
	
}

s32 Audio_TellXA_Sector(void)
{
	return 0;
}

s32 Audio_TellXA_Milli(void)
{
	return (s32)((glfwGetTime() - xa_time) * 1000.0);
}

boolean Audio_PlayingXA(void)
{
	return true;
}

void Audio_WaitPlayXA(void)
{
	
}

void Audio_ProcessXA(void)
{
	
}
