#include "../audio.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

//Audio functions
void Audio_Init(void)
{
	
}

void Audio_GetXAFile(CdlFILE *file, XA_Track track)
{
	
}

void Audio_PlayXA_File(CdlFILE *file, u8 volume, u8 channel, boolean loop)
{
	
}

void Audio_PlayXA_Track(XA_Track track, u8 volume, u8 channel, boolean loop)
{
	
}

void Audio_PlayXA(const char *path, u8 volume, u8 channel, boolean loop)
{
	
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
	return (s32)(glfwGetTime() * 1000.0);
}

boolean Audio_PlayingXA(void)
{
	return false;
}

void Audio_WaitPlayXA(void)
{
	
}

void Audio_ProcessXA(void)
{
	
}
