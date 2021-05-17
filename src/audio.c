#include "psx.h"

#include "audio.h"
#include "io.h"
#include "main.h"

//XA state
static int xa_active, xa_loop, xa_start, xa_end, xa_pos;
static void *xa_readycb;

static void XA_ReadyCallback(u_char intr, u_char *result)
{
	static u_char chk_cool = 0;
	
	if (intr == CdlDataReady)
	{
		//Check if we've reached end
		if ((chk_cool++ & 0xF) == 0 && Audio_TellXA_Sector() >= (xa_end - xa_start))
		{
			if (xa_loop)
			{
				//Reset XA playback
				CdlLOC loc;
				CdIntToPos(xa_start, &loc);
				CdControlF(CdlReadN, (u_char*)&loc);
				xa_pos = xa_start;
			}
			else
			{
				//Stop XA playback
				Audio_StopXA();
			}
		}
	}
}

//Audio functions
void Audio_Init()
{
	//Initialize audio
	SsInit();
	
	//Initialize XA state
	xa_active = 0;
}

void Audio_PlayXA_Pos(int start, int end, int volume, int channel, int loop)
{
	//Use input
	xa_pos = start;
	xa_start = start;
	xa_end = end;
	SsSetSerialVol(SS_SERIAL_A, volume, volume);
	xa_loop = loop;
	
	//Prepare CD for XA reading
	u_char param[4];
	param[0] = CdlModeSpeed | CdlModeRT | CdlModeSF;
	
	CdControlB(CdlSetmode, param, 0);
	CdControlF(CdlPause, 0);
	xa_readycb = CdReadyCallback(XA_ReadyCallback);
	
	//Play XA
	Audio_ChannelXA(channel);
	
	CdlLOC loc;
	CdIntToPos(start, &loc);
	CdControlF(CdlReadN, (u_char*)&loc);
	xa_active = 1;
}

void Audio_PlayXA(const char *path, int volume, int channel, int loop)
{
	//Search for file
	CdlFILE file;
	if (!CdSearchFile(&file, (char*)path))
	{
		sprintf(error_msg, "[Audio_PlayXA] %s not found", path);
		ErrorLock();
		return;
	}
	
	//Play file
	int start = CdPosToInt(&file.pos);
	Audio_PlayXA_Pos(start, start + (file.size / IO_SECT_SIZE) - 1, volume, channel, loop);
}

void Audio_ChannelXA(int channel)
{
	//Change CD filter
	CdlFILTER filter;
	filter.file = 1;
	filter.chan = channel;
	
	CdControlF(CdlSetfilter, (u_char*)&filter);
}

static u_char Audio_FromBCD(u_char x)
{
	return x - 6 * (x >> 4);
}

int Audio_TellXA_Sector()
{
	if (!xa_active)
		return -1;
	u_char result[8];
	CdControlB(CdlGetlocP, NULL, result);
	return ((Audio_FromBCD(result[2]) * 75 * 60) + (Audio_FromBCD(result[3]) * 75) + Audio_FromBCD(result[4])) - xa_start;
}

int Audio_TellXA_Milli()
{
	if (!xa_active)
		return -1;
	return Audio_TellXA_Sector() * 1000 / 150;
}

int Audio_PlayingXA()
{
	return xa_active;
}

void Audio_StopXA()
{
	if (xa_active)
	{
		//Reset XA state
		xa_active = 0;
		
		//Reset CD state
		CdReadyCallback(xa_readycb);
		CdControlF(CdlStop,0);
		
		u_char param[4];
		param[0] = CdlModeSpeed;
		CdControlB(CdlSetmode, param, 0);
	}
}
