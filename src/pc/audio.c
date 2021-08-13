#include "../audio.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

double xa_time;

//Audio functions
void Audio_Init(void)
{
	
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
