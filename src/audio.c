#include "psx.h"

#include "audio.h"
#include "io.h"
#include "main.h"

#include "pad.h"

//XA state
#define XA_STATE_INIT    (1 << 0)
#define XA_STATE_PLAYING (1 << 1)
#define XA_STATE_LOOPS   (1 << 2)
#define XA_STATE_SEEKING    (1 << 3)
static u8 xa_state;
static u32 xa_pos, xa_start, xa_end;

//Internal XA functions
static u8 XA_BCD(u8 x)
{
	return x - 6 * (x >> 4);
}

static u32 XA_TellSector()
{
	u8 result[8];
	CdControlB(CdlGetlocP, NULL, result);
	return (XA_BCD(result[2]) * 75 * 60) + (XA_BCD(result[3]) * 75) + XA_BCD(result[4]);
}

static void XA_SetVolume(u8 x)
{
	//Set CD mix volume
	CdlATV cd_vol;
	cd_vol.val0 = cd_vol.val1 = cd_vol.val2 = cd_vol.val3 = x;
	CdMix(&cd_vol);
}

static void XA_Init()
{
	u8 param[4];
	
	//Set XA state
	if (xa_state & XA_STATE_INIT)
		return;
	xa_state = XA_STATE_INIT;
	
	//Set CD mix flag
	SpuCommonAttr spu_attr;
	spu_attr.mask = SPU_COMMON_CDMIX | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR;
	spu_attr.cd.mix = SPU_ON;
	spu_attr.cd.volume.left = spu_attr.cd.volume.right  = 0x7FFF; //Can't explain this magic number
	SpuSetCommonAttr(&spu_attr);
	
	//Set initial volume
	XA_SetVolume(0);
	
	//Prepare CD drive for XA reading
	param[0] = CdlModeRT | CdlModeSF | CdlModeSize1;
	
	CdControlB(CdlSetmode, param, 0);
	CdControlF(CdlPause, 0);
}

static void XA_Quit()
{
	//Set XA state
	if (!(xa_state & XA_STATE_INIT))
		return;
	xa_state = 0;
	
	//Stop playing XA
	XA_SetVolume(0);
	CdControlB(CdlPause, 0, 0);
}

static void XA_Play(u32 start)
{
	//Play at given position
	CdlLOC cd_loc;
	CdIntToPos(start, &cd_loc);
	CdControlF(CdlReadS, (u8*)&cd_loc);
}

static void XA_Pause()
{
	//Set XA state
	if (!(xa_state & XA_STATE_PLAYING))
		return;
	xa_state &= ~XA_STATE_PLAYING;
	
	//Pause playback
	CdControlB(CdlPause, 0, 0);
}

static void XA_SetFilter(u8 channel)
{
	//Change CD filter
	CdlFILTER filter;
	filter.file = 1;
	filter.chan = channel;
	CdControlF(CdlSetfilter, (u8*)&filter);
}

//Audio functions
void Audio_Init()
{
	//Initialize sound system
	SsInit();
	
	//Set XA state
	xa_state = 0;
}

void Audio_PlayXA_Pos(u32 start, u32 end, u8 volume, u8 channel, boolean loop)
{
	//Initialize XA system and stop previous song
	XA_Init();
	XA_SetVolume(0);
	
	//Set XA state
	xa_start = xa_pos = start;
	xa_end = end;
	xa_state = XA_STATE_INIT | XA_STATE_PLAYING | XA_STATE_SEEKING;
	if (loop)
		xa_state |= XA_STATE_LOOPS;
	
	//Start seeking to XA and use parameters
	CdlLOC cd_loc;
	CdIntToPos(start, &cd_loc);
	CdControlB(CdlSeekL, (u8*)&cd_loc, 0);
	
	XA_SetFilter(channel);
	XA_SetVolume(volume);
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
	u32 start = CdPosToInt(&file.pos);
	Audio_PlayXA_Pos(start, start + (file.size / IO_SECT_SIZE) - 1, volume, channel, loop);
}

void Audio_PauseXA()
{
	//Pause playing XA file
	XA_Pause();
}

void Audio_StopXA()
{
	//Deinitialize XA system
	XA_Quit();
}

void Audio_ChannelXA(u8 channel)
{
	//Set XA filter to the given channel
	XA_SetFilter(channel);
}

s32 Audio_TellXA_Sector()
{
	//Get CD position
	return (s32)xa_pos - (s32)xa_start; //Meh casting
}

s32 Audio_TellXA_Milli()
{
	return ((s32)xa_pos - (s32)xa_start) * 1000 / 75; //1000 / (75 * speed (1x))
}

boolean Audio_PlayingXA()
{
	return (xa_state & XA_STATE_PLAYING) != 0;
}

void Audio_ProcessXA()
{
	//Handle playing state
	if (xa_state & XA_STATE_PLAYING)
	{
		//Handle seeking state
		if (xa_state & XA_STATE_SEEKING)
		{
			//Check if CD is still seeking to the XA's beginning
			if (!(CdStatus() & CdlStatSeek))
			{
				//Stopped seeking
				xa_state &= ~XA_STATE_SEEKING;
				XA_Play(xa_start);
			}
			else
			{
				//Still seeking
				return;
			}
		}
		
		//Get CD position
		xa_pos = XA_TellSector();
		
		//Check position
		if (xa_pos >= xa_end)
		{
			if (xa_state & XA_STATE_LOOPS)
			{
				//Reset XA playback
				CdlLOC cd_loc;
				CdIntToPos(xa_pos = xa_start, &cd_loc);
				CdControlB(CdlSeekL, (u8*)&cd_loc, 0);
				xa_state |= XA_STATE_SEEKING;
			}
			else
			{
				//Stop XA playback
				Audio_StopXA();
			}
		}
	}
}

