/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*
  The bulk of this code was written by spicyjpeg
  (C) 2021 spicyjpeg
*/

#include "../audio.h"

#include "../timer.h"
#include "../io.h"

//Audio constants
#define SAMPLE_RATE 0x1000 //44100 Hz

#define BUFFER_SIZE (13 << 11) //13 sectors
#define CHUNK_SIZE (BUFFER_SIZE * audio_streamcontext.header.s.channels)

#define BUFFER_TIME FIXED_DEC(((BUFFER_SIZE * 28) / 16), 44100)

#define BUFFER_START_ADDR 0x1010

//SPU registers
typedef struct
{
	u16 vol_left;
	u16 vol_right;
	u16 freq;
	u16 addr;
	u32 adsr_param;
	u16 _reserved;
	u16 loop_addr;
} Audio_SPUChannel;

#define SPU_CTRL     *((volatile u16*)0x1f801daa)
#define SPU_DMA_CTRL *((volatile u16*)0x1f801dac)
#define SPU_IRQ_ADDR *((volatile u16*)0x1f801da4)
#define SPU_KEY_ON   *((volatile u32*)0x1f801d88)
#define SPU_KEY_OFF  *((volatile u32*)0x1f801d8c)

#define SPU_CHANNELS    ((volatile Audio_SPUChannel*)0x1f801c00)
#define SPU_RAM_ADDR(x) ((u16)(((u32)(x)) >> 3))

//Audio streaming
typedef struct
{
	//CD state
	enum
	{
		Audio_StreamState_Stopped,
		Audio_StreamState_Ini,
		Audio_StreamState_Play,
		Audio_StreamState_Playing,
	} state;
	boolean loops;
	
	u32 cd_lba;
	u32 cd_length;
	u32 cd_pos;
	
	//SPU state
	u32 spu_addr;
	u32 spu_pos;
	u32 spu_swap;
	
	//Timing state
	u32 timing_chunk;
	fixed_t timing_pos, timing_start;
	
	//Header
	union
	{
		struct
		{
			u8 channels;
		} s;
		u8 d[2048];
	} header;
} Audio_StreamContext;

static volatile Audio_StreamContext audio_streamcontext;

void Audio_StreamIRQ_SPU(void)
{
	//Disable SPU IRQ until we've finished streaming more data
	SpuSetIRQ(SPU_OFF);
	
	//Don't run if stopped
	if (audio_streamcontext.state == Audio_StreamState_Stopped)
		return;
	
	//Update timing state
	audio_streamcontext.timing_chunk++;
	audio_streamcontext.timing_pos = (audio_streamcontext.timing_chunk << FIXED_SHIFT) * (BUFFER_SIZE / 16) / 1575;
	audio_streamcontext.timing_start = timer_sec;
	
	//Update addresses
	if ((audio_streamcontext.spu_swap ^= 1) != 0)
		audio_streamcontext.spu_addr = BUFFER_START_ADDR + CHUNK_SIZE;
	else
		audio_streamcontext.spu_addr = BUFFER_START_ADDR;
	audio_streamcontext.spu_pos = 0;
	
	for (int i = 0; i < audio_streamcontext.header.s.channels; i++)
		SPU_CHANNELS[i].loop_addr = SPU_RAM_ADDR(audio_streamcontext.spu_addr + BUFFER_SIZE * i);
	
	//Check for loop
	if (audio_streamcontext.loops)
	{
		//Return to beginning of mus
		if (audio_streamcontext.cd_pos >= audio_streamcontext.cd_length)
		{
			audio_streamcontext.timing_pos = 0;
			audio_streamcontext.cd_pos = 0;
		}
	}
	else
	{
		//Stop playing
		if (audio_streamcontext.cd_pos == audio_streamcontext.cd_length)
		{
			//Continue streaming from CD (wrap to prevent unintended errors)
			CdlLOC pos;
			CdIntToPos(audio_streamcontext.cd_lba, &pos);
			CdControlF(CdlReadN, (u8*)&pos);
			return;
		}
		else if (audio_streamcontext.cd_pos > audio_streamcontext.cd_length)
		{
			//Stop playing
			Audio_StopMus();
			return;
		}
	}
	
	//Continue streaming from CD
	CdlLOC pos;
	CdIntToPos(audio_streamcontext.cd_lba + audio_streamcontext.cd_pos, &pos);
	CdControlF(CdlReadN, (u8*)&pos);
}

static u8 read_sector[2048];

void Audio_StreamIRQ_CD(u8 event, u8 *payload)
{
	(void)payload;
	
	//Don't run if stopped
	if (audio_streamcontext.state == Audio_StreamState_Stopped)
	{
		CdControlF(CdlPause, NULL);
		return;
	}
	
	//Ignore all events other than a sector being ready
	if (event != CdlDataReady)
		return;
	
	//Fetch the sector that has been read from the drive
	CdGetSector(read_sector, 2048 / 4);
	audio_streamcontext.cd_pos++;
	
	//DMA to SPU
	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	SpuSetTransferStartAddr(audio_streamcontext.spu_addr + audio_streamcontext.spu_pos);
	audio_streamcontext.spu_pos += 2048;
	
	SpuWrite(read_sector, 2048);
	
	//Start SPU IRQ if finished reading
	if (audio_streamcontext.spu_pos >= CHUNK_SIZE)
	{
		switch (audio_streamcontext.state)
		{
			case Audio_StreamState_Ini:
			{
				//Update addresses
				audio_streamcontext.spu_addr = BUFFER_START_ADDR + CHUNK_SIZE;
				audio_streamcontext.spu_swap = 1;
				audio_streamcontext.spu_pos = 0;
				
				//Set state
				audio_streamcontext.state = Audio_StreamState_Play;
				break;
			}
			case Audio_StreamState_Play:
			{
				//Stop and turn on SPU IRQ
				CdControlF(CdlPause, NULL);
				SpuSetIRQAddr(audio_streamcontext.spu_addr);
				SpuSetIRQ(SPU_ON);
				
				//Set state
				audio_streamcontext.state = Audio_StreamState_Playing;
				break;
			}
			case Audio_StreamState_Playing:
			{
				//Stop and turn on SPU IRQ
				CdControlF(CdlPause, NULL);
				SpuSetIRQAddr(audio_streamcontext.spu_addr);
				SpuSetIRQ(SPU_ON);
				break;
			}
			default:
				break;
		}
	}
}

//Audio interface
void Audio_Init(void)
{
	//Initialize SPU
	SpuInit();
	
	//Set SPU common attributes
	SpuCommonAttr spu_attr;
	spu_attr.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR;
	spu_attr.mvol.left  = 0x3FFF;
	spu_attr.mvol.right = 0x3FFF;
	SpuSetCommonAttr(&spu_attr);
	
	//Reset context
	audio_streamcontext.timing_start = -1;
}

void Audio_Quit(void)
{
	
}

void Audio_Reset(void)
{
	//Reset SPU
	SpuSetIRQCallback(NULL);
	SpuSetIRQ(SPU_OFF);
	
	//Upload dummy block at end of stream
	u32 dummy_addr = BUFFER_START_ADDR + (CHUNK_SIZE * 2);
	static u8 dummy[64] = {0, 5};
	
	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	SpuSetTransferStartAddr(dummy_addr);
	SpuWrite(dummy, sizeof(dummy));
	
	SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
	
	//Reset keys
	for (int i = 0; i < 24; i++)
	{
		SPU_CHANNELS[i].vol_left   = 0x0000;
		SPU_CHANNELS[i].vol_right  = 0x0000;
		SPU_CHANNELS[i].addr       = SPU_RAM_ADDR(dummy_addr);
		SPU_CHANNELS[i].freq       = 0;
		SPU_CHANNELS[i].adsr_param = 0x9FC080FF;
	}
	SPU_KEY_OFF |= 0x00FFFFFF;
	SPU_KEY_ON |= 0x00FFFFFF;
}

void Audio_LoadMusFile(CdlFILE *file)
{
	//Stop playing mus
	Audio_StopMus();
	
	//Read header
	CdReadyCallback(NULL);
	CdControl(CdlSetloc, (u8*)&file->pos, NULL);
	CdRead(1, (IO_Data)audio_streamcontext.header.d, CdlModeSpeed);
	CdReadSync(0, NULL);
	
	//Reset context
	audio_streamcontext.state = Audio_StreamState_Ini;
	
	audio_streamcontext.timing_chunk = 0;
	audio_streamcontext.timing_pos = 0;
	audio_streamcontext.timing_start = -1;
	
	audio_streamcontext.spu_addr = BUFFER_START_ADDR;
	audio_streamcontext.spu_swap = 0;
	audio_streamcontext.spu_pos = 0;
	
	//Use mus file
	audio_streamcontext.cd_lba = CdPosToInt(&file->pos) + 1;
	audio_streamcontext.cd_length = ((file->size + 2047) >> 11) - 1;
	audio_streamcontext.cd_pos = 0;
	
	//Setup SPU
	Audio_Reset();
	SpuSetIRQCallback(Audio_StreamIRQ_SPU);
	
	//Begin streaming from CD
	u8 param[4];
	param[0] = CdlModeSpeed;
	CdControlB(CdlSetmode, param, 0);
	
	CdlLOC pos;
	CdIntToPos(audio_streamcontext.cd_lba + audio_streamcontext.cd_pos, &pos);
	
	CdReadyCallback(Audio_StreamIRQ_CD);
	CdControlF(CdlReadN, (u8*)&pos);
}

void Audio_LoadMus(const char *path)
{
	//Find requested file
	CdlFILE file;
	IO_FindFile(&file, path);
	
	//Load found file
	Audio_LoadMusFile(&file);
}

void Audio_PlayMus(boolean loops)
{
	//Wait for play state
	while (audio_streamcontext.state != Audio_StreamState_Playing)
		__asm__("nop");
	
	//Start timing
	audio_streamcontext.timing_chunk = 0;
	audio_streamcontext.timing_pos = 0;
	audio_streamcontext.timing_start = timer_sec;
	
	//Play keys
	audio_streamcontext.loops = loops;
	
	u16 key_or = 0;
	for (int i = 0; i < audio_streamcontext.header.s.channels; i++)
	{
		SPU_CHANNELS[i].addr       = SPU_RAM_ADDR(BUFFER_START_ADDR + BUFFER_SIZE * i);
		SPU_CHANNELS[i].loop_addr  = SPU_CHANNELS[i].addr + SPU_RAM_ADDR(CHUNK_SIZE);
		SPU_CHANNELS[i].freq       = SAMPLE_RATE;
		SPU_CHANNELS[i].adsr_param = 0x9FC080FF;
		key_or |= (1 << i);
	}
	SPU_KEY_ON |= key_or;
}

void Audio_StopMus(void)
{
	//Reset context
	audio_streamcontext.state = Audio_StreamState_Stopped;
	
	//Reset keys
	u32 dummy_addr = BUFFER_START_ADDR + (CHUNK_SIZE * 2);
	
	for (int i = 0; i < 24; i++)
	{
		SPU_CHANNELS[i].vol_left   = 0x0000;
		SPU_CHANNELS[i].vol_right  = 0x0000;
		SPU_CHANNELS[i].addr       = SPU_RAM_ADDR(dummy_addr);
		SPU_CHANNELS[i].freq       = 0;
		SPU_CHANNELS[i].adsr_param = 0x9FC080FF;
	}
	SPU_KEY_OFF |= 0x00FFFFFF;
	SPU_KEY_ON |= 0x00FFFFFF;
	
	//Reset SPU
	SpuSetIRQCallback(NULL);
	SpuSetIRQ(SPU_OFF);
	
	//Reset CD
	CdReadyCallback(NULL);
	CdControlF(CdlPause, NULL);
}

void Audio_SetVolume(u8 i, u16 vol_left, u16 vol_right)
{
	SPU_CHANNELS[i].vol_left = vol_left;
	SPU_CHANNELS[i].vol_right = vol_right;
}

fixed_t Audio_GetTime(void)
{
	if (audio_streamcontext.timing_pos < 0 || audio_streamcontext.timing_start < 0)
		return 0;
	fixed_t dt = timer_sec - audio_streamcontext.timing_start;
	if (dt > BUFFER_TIME)
		return audio_streamcontext.timing_pos + BUFFER_TIME;
	return audio_streamcontext.timing_pos + dt;
}

boolean Audio_IsPlaying(void)
{
	return audio_streamcontext.state != Audio_StreamState_Stopped;
}
