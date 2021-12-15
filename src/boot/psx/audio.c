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

#define BUFFER_SIZE (13 << 11) //13 sectors = 26624 bytes = 1.05 seconds (see BUFFER_TIME)
#define CHUNK_SIZE (BUFFER_SIZE * 2)

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
	u32 cd_lba;
	u32 cd_length;
	u32 cd_pos;
	
	//SPU state
	u32 spu_addr;
	u32 spu_pos;
	u32 spu_swap;
	
	//Timing state
	fixed_t timing_pos, timing_start;
} Audio_StreamContext;

static volatile Audio_StreamContext audio_streamcontext;

void Audio_StreamIRQ_SPU(void)
{
	//Disable SPU IRQ until we've finished streaming more data
	SpuSetIRQ(SPU_OFF);
	
	//Update timing state
	audio_streamcontext.timing_pos += BUFFER_TIME;
	audio_streamcontext.timing_start = timer_sec;
	
	//Swap active buffer
	audio_streamcontext.spu_swap ^= 1;
	audio_streamcontext.spu_pos = 0;
	
	//Update addresses
	audio_streamcontext.spu_addr = BUFFER_START_ADDR + CHUNK_SIZE * audio_streamcontext.spu_swap;
	
	SPU_CHANNELS[0].loop_addr = SPU_RAM_ADDR(audio_streamcontext.spu_addr);
	SPU_CHANNELS[1].loop_addr = SPU_RAM_ADDR(audio_streamcontext.spu_addr + BUFFER_SIZE);
	
	//Unmute the channels
	SPU_CHANNELS[0].vol_left  = 0x3FFF;
	SPU_CHANNELS[1].vol_right = 0x3FFF;
	
	//Continue streaming from CD
	CdlLOC pos;
	CdIntToPos(audio_streamcontext.cd_lba + audio_streamcontext.cd_pos, &pos);
	CdControlF(CdlReadN, (u8*)&pos);
}

void Audio_StreamIRQ_CD(u8 event, u8 *payload)
{
	(void)payload;
	
	//Ignore all events other than a sector being ready
	if (event != CdlDataReady)
		return;
	
	//Fetch the sector that has been read from the drive
	static u8 sector[2048];
	CdGetSector(sector, 2048 / 4);
	audio_streamcontext.cd_pos++;
	
	//DMA to SPU
	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	SpuSetTransferStartAddr(audio_streamcontext.spu_addr + audio_streamcontext.spu_pos);
	audio_streamcontext.spu_pos += 2048;
	
	SpuWrite(sector, 2048);
	
	//Start SPU IRQ if finished reading
	if (audio_streamcontext.spu_pos >= CHUNK_SIZE)
	{
		CdControlF(CdlPause, NULL);
		SpuSetIRQAddr(audio_streamcontext.spu_addr);
		SpuSetIRQ(SPU_ON);
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

void Audio_Reset(u32 stream_size)
{
	//Reset callbacks
	SpuSetIRQCallback(NULL);
	
	//Upload dummy block at end of stream
	u32 dummy_addr = BUFFER_START_ADDR + stream_size;
	static u8 dummy[64] = {0, 5};
	
	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	SpuSetTransferStartAddr(dummy_addr);
	SpuWrite(dummy, sizeof(dummy));
	
	SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
	
	//Reset keys
	SPU_KEY_OFF = 0x00FFFFFF;
	for (int i = 0; i < 24; i++)
	{
		SPU_CHANNELS[i].vol_left   = 0x0000;
		SPU_CHANNELS[i].vol_right  = 0x0000;
		SPU_CHANNELS[i].addr       = SPU_RAM_ADDR(dummy_addr);
		SPU_CHANNELS[i].freq       = 0;
		SPU_CHANNELS[i].adsr_param = 0x9fc080ff;
	}
}

void Audio_PlayMusFile(CdlFILE *file)
{
	//Stop playing mus
	Audio_StopMus();
	
	//Reset context
	audio_streamcontext.timing_pos = BUFFER_TIME * -2;
	audio_streamcontext.timing_start = -1;
	
	//Setup CD
	u8 param[4];
	param[0] = CdlModeSpeed;
	CdControlB(CdlSetmode, param, 0);
	CdReadyCallback(Audio_StreamIRQ_CD);
	
	//Setup SPU
	Audio_Reset(CHUNK_SIZE * 2);
	SpuSetIRQCallback(Audio_StreamIRQ_SPU);
	
	//Initialize context
	audio_streamcontext.cd_lba = CdPosToInt(&file->pos);
	audio_streamcontext.cd_length = (file->size + 2047) >> 11;
	audio_streamcontext.cd_pos = 0;
	
	//Preload chunk
	audio_streamcontext.spu_swap = 0;
	Audio_StreamIRQ_SPU();
	
	while (audio_streamcontext.spu_pos < CHUNK_SIZE)
		__asm__("nop");
	
	//Start playing channels 0 and 1
	for (int i = 0; i < 2; i++)
	{
		SPU_CHANNELS[i].vol_left   = 0x0000;
		SPU_CHANNELS[i].vol_right  = 0x0000;
		SPU_CHANNELS[i].addr       = SPU_RAM_ADDR(BUFFER_START_ADDR + BUFFER_SIZE * i);
		SPU_CHANNELS[i].freq       = SAMPLE_RATE;
		SPU_CHANNELS[i].adsr_param = 0xDFF18087;
	}
	SPU_KEY_ON |= 0x0003;
	
	Audio_StreamIRQ_SPU();
}

void Audio_PlayMus(const char *path)
{
	//Find requested file
	CdlFILE file;
	IO_FindFile(&file, path);
	
	//Play found file
	Audio_PlayMusFile(&file);
}

void Audio_StopMus(void)
{
	//Stop CD, callbacks, and keys
	CdReadyCallback(NULL);
	SpuSetIRQCallback(NULL);
	SPU_KEY_OFF = 0x00FFFFFF;
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