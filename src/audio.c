#include "psx.h"

#include "audio.h"
#include "io.h"
#include "main.h"

//XA state
static boolean xa_active, xa_loop;
static s32 xa_start, xa_end, xa_pos;
static void *xa_readycb;

static void XA_ReadyCallback(u8 intr, u8 *result)
{
	static u8 chk_cool = 0;
	
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
				CdControlF(CdlReadN, (u8*)&loc);
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

void Audio_PlayXA_Pos(s32 start, s32 end, u8 volume, u8 channel, boolean loop)
{
	//Use input
	xa_pos = start;
	xa_start = start;
	xa_end = end;
	SsSetSerialVol(SS_SERIAL_A, volume, volume);
	xa_loop = loop;
	
	//Prepare CD for XA reading
	u8 param[4];
	param[0] = CdlModeSpeed | CdlModeRT | CdlModeSF;
	
	CdControlB(CdlSetmode, param, 0);
	CdControlF(CdlPause, 0);
	xa_readycb = CdReadyCallback(XA_ReadyCallback);
	
	//Play XA
	Audio_ChannelXA(channel);
	
	CdlLOC loc;
	CdIntToPos(start, &loc);
	CdControlF(CdlReadN, (u8*)&loc);
	xa_active = 1;
}

void Audio_PlayXA(const char *path, u8 volume, u8 channel, boolean loop)
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
	s32 start = CdPosToInt(&file.pos);
	Audio_PlayXA_Pos(start, start + (file.size / IO_SECT_SIZE) - 1, volume, channel, loop);
}

void Audio_ChannelXA(u8 channel)
{
	//Change CD filter
	CdlFILTER filter;
	filter.file = 1;
	filter.chan = channel;
	CdControlF(CdlSetfilter, (u8*)&filter);
}

static u8 Audio_FromBCD(u8 x)
{
	return x - 6 * (x >> 4);
}

s32 Audio_TellXA_Sector()
{
	if (!xa_active)
		return -1;
	u8 result[8];
	CdControlB(CdlGetlocP, NULL, result);
	return ((Audio_FromBCD(result[2]) * 75 * 60) + (Audio_FromBCD(result[3]) * 75) + Audio_FromBCD(result[4])) - xa_start;
}

s32 Audio_TellXA_Milli()
{
	if (!xa_active)
		return -1;
	return Audio_TellXA_Sector() * 1000 / 150;
}

boolean Audio_PlayingXA()
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
		
		u8 param[4];
		param[0] = CdlModeSpeed;
		CdControlB(CdlSetmode, param, 0);
	}
}
