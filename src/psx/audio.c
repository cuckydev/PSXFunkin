/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../audio.h"

#include "../io.h"
#include "../main.h"

//XA state
#define XA_STATE_INIT    (1 << 0)
#define XA_STATE_PLAYING (1 << 1)
#define XA_STATE_LOOPS   (1 << 2)
#define XA_STATE_SEEKING (1 << 3)
static u8 xa_state, xa_resync, xa_volume, xa_channel;
static u32 xa_pos, xa_start, xa_end;

//XA files and tracks
static CdlFILE xa_files[XA_Max];

#include "../audio_def.h"

//Internal XA functions
static u8 XA_BCD(u8 x)
{
	return x - 6 * (x >> 4);
}

static u32 XA_TellSector(void)
{
	u8 result[8];
	CdControlB(CdlGetlocP, NULL, result);
	return (XA_BCD(result[2]) * 75 * 60) + (XA_BCD(result[3]) * 75) + XA_BCD(result[4]);
}

static void XA_SetVolume(u8 x)
{
	//Set CD mix volume
	CdlATV cd_vol;
	xa_volume = cd_vol.val0 = cd_vol.val1 = cd_vol.val2 = cd_vol.val3 = x;
	CdMix(&cd_vol);
}

static void XA_Init(void)
{
	u8 param[4];
	
	//Set XA state
	if (xa_state & XA_STATE_INIT)
		return;
	xa_state = XA_STATE_INIT;
	xa_resync = 0;
	
	//Set CD mix flag
	SpuCommonAttr spu_attr;
	spu_attr.mask = SPU_COMMON_CDMIX | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR;
	spu_attr.cd.mix = SPU_ON;
	spu_attr.cd.volume.left = spu_attr.cd.volume.right = 0x6000; //Lame magic number
	SpuSetCommonAttr(&spu_attr);
	
	//Set initial volume
	XA_SetVolume(0);
	
	//Prepare CD drive for XA reading
	param[0] = CdlModeRT | CdlModeSF | CdlModeSize1;
	
	CdControlB(CdlSetmode, param, NULL);
	CdControlF(CdlPause, NULL);
}

static void XA_Quit(void)
{
	//Set XA state
	if (!(xa_state & XA_STATE_INIT))
		return;
	xa_state = 0;
	
	//Stop playing XA
	XA_SetVolume(0);
	CdControlB(CdlPause, NULL, NULL);
}

static void XA_Play(u32 start)
{
	//Play at given position
	CdlLOC cd_loc;
	CdIntToPos(start, &cd_loc);
	CdControlF(CdlReadS, (u8*)&cd_loc);
}

static void XA_Pause(void)
{
	//Set XA state
	if (!(xa_state & XA_STATE_PLAYING))
		return;
	xa_state &= ~XA_STATE_PLAYING;
	
	//Pause playback
	CdControlB(CdlPause, NULL, NULL);
}

static void XA_SetFilter(u8 channel)
{
	//Change CD filter
	CdlFILTER filter;
	filter.file = 1;
	xa_channel = filter.chan = channel;
	CdControlF(CdlSetfilter, (u8*)&filter);
}

//Audio functions
void Audio_Init(void)
{
	//Initialize sound system
	SsInit();
	SsSetSerialVol(SS_SERIAL_A, 0x7F, 0x7F);
	
	//Set XA state
	xa_state = 0;
	
	//Get file positions
	CdlFILE *filep = xa_files;
	for (const char **pathp = xa_paths; *pathp != NULL; pathp++)
		IO_FindFile(filep++, *pathp);
}

void Audio_Quit(void)
{
	
}

static void Audio_GetXAFile(CdlFILE *file, XA_Track track)
{
	const XA_TrackDef *track_def = &xa_tracks[track];
	file->pos = xa_files[track_def->file].pos;
	file->size = track_def->length;
}

static void Audio_PlayXA_File(CdlFILE *file, u8 volume, u8 channel, boolean loop)
{
	//Initialize XA system and stop previous song
	XA_Init();
	XA_SetVolume(0);
	
	//Set XA state
	xa_start = xa_pos = CdPosToInt(&file->pos);
	xa_end = xa_start + (file->size / IO_SECT_SIZE) - 1;
	xa_state = XA_STATE_INIT | XA_STATE_PLAYING | XA_STATE_SEEKING;
	xa_resync = 0;
	if (loop)
		xa_state |= XA_STATE_LOOPS;
	
	//Start seeking to XA and use parameters
	IO_SeekFile(file);
	XA_SetFilter(channel);
	XA_SetVolume(volume);
}

void Audio_PlayXA_Track(XA_Track track, u8 volume, u8 channel, boolean loop)
{
	//Get track information
	CdlFILE file;
	Audio_GetXAFile(&file, track);
	
	//Play track
	Audio_PlayXA_File(&file, volume, channel, loop);
}

void Audio_SeekXA_Track(XA_Track track)
{
	//Get track file and seek
	CdlFILE file;
	Audio_GetXAFile(&file, track);
	IO_SeekFile(&file);
}

void Audio_PauseXA(void)
{
	//Pause playing XA file
	XA_Pause();
}

void Audio_StopXA(void)
{
	//Deinitialize XA system
	XA_Quit();
}

void Audio_ChannelXA(u8 channel)
{
	//Set XA filter to the given channel
	XA_SetFilter(channel);
}

s32 Audio_TellXA_Sector(void)
{
	//Get CD position
	return (s32)xa_pos - (s32)xa_start; //Meh casting
}

s32 Audio_TellXA_Milli(void)
{
	return ((s32)xa_pos - (s32)xa_start) * 1000 / 75; //1000 / (75 * speed (1x))
}

boolean Audio_PlayingXA(void)
{
	return (xa_state & XA_STATE_PLAYING) != 0;
}

void Audio_WaitPlayXA(void)
{
	while (1)
	{
		Audio_ProcessXA();
		if (Audio_PlayingXA())
			return;
		VSync(0);
	}
}

void Audio_ProcessXA(void)
{
	//Handle playing state
	if (xa_state & XA_STATE_PLAYING)
	{
		//Retrieve CD status
		CdControl(CdlNop, NULL, NULL);
		u8 cd_status = CdStatus();
		
		//Handle resync timer
		if (xa_resync != 0)
		{
			//Wait for resync timer
			if (--xa_resync != 0)
				return;
			
			//Check if we're in a proper state
			if (cd_status & CdlStatShellOpen)
				return;
			
			//Attempt to get CD drive active
			while (1)
			{
				CdControl(CdlNop, NULL, NULL);
				cd_status = CdStatus();
				if (cd_status & CdlStatStandby)
					break;
			}
			
			//Re-initialize XA system
			u8 prev_state = xa_state;
			XA_Init();
			xa_state = prev_state;
			
			XA_SetFilter(xa_channel);
			XA_SetVolume(xa_volume);
			
			//Get new CD status
			CdControl(CdlNop, NULL, NULL);
			cd_status = CdStatus();
		}
		
		//Check CD status for issues
		if (cd_status & CdlStatShellOpen)
		{
			//Seek just ahead of last reported valid position
			if (!(xa_state & XA_STATE_SEEKING))
			{
				xa_pos++;
				xa_state |= XA_STATE_SEEKING;
			}
			
			//Wait a moment before attempting the actual resync
			xa_resync = 60;
			return;
		}
		
		//Handle seeking state
		if (xa_state & XA_STATE_SEEKING)
		{
			//Check if CD is still seeking to the XA's beginning
			if (!(cd_status & CdlStatSeek))
			{
				//Stopped seeking
				xa_state &= ~XA_STATE_SEEKING;
				XA_Play(xa_pos);
			}
			else
			{
				//Still seeking
				return;
			}
		}
		
		//Get CD position
		u32 next_pos = XA_TellSector();
		if (next_pos > xa_pos)
			xa_pos = next_pos;
		
		//Check position
		if (xa_pos >= xa_end)
		{
			if (xa_state & XA_STATE_LOOPS)
			{
				//Reset XA playback
				CdlLOC cd_loc;
				CdIntToPos(xa_pos = xa_start, &cd_loc);
				CdControlB(CdlSeekL, (u8*)&cd_loc, NULL);
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

