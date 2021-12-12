/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../audio.h"

//Function declarations
extern void InterruptCallback(int index, void (*cb)(void));

//Audio constants
#define SAMPLE_RATE 0x1000 //44100 Hz
#define BUFFER_SIZE (13 << 11) //26624 bytes = 1.05 seconds

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
#define SPU_IRQ_ADDR *((volatile u16*)0x1f801da4)
#define SPU_KEY_ON   *((volatile u32*)0x1f801d88)
#define SPU_KEY_OFF  *((volatile u32*)0x1f801d8c)

#define SPU_CHANNELS    ((volatile Audio_SPUChannel*)0x1f801c00)
#define SPU_RAM_ADDR(x) ((u16)(((u32)(x)) >> 3))

//Audio streaming
#define BUFFER_START_ADDR 0x1000
#define CHUNK_SIZE        (BUFFER_SIZE * 2)

typedef struct
{
	//CD state
	u32 lba;
	u32 length;
	u32 pos;
	
	//SPU state
	u32 spu_addr;
	u32 spu_pos;
	u32 db_active;
} Audio_StreamContext;

static volatile Audio_StreamContext audio_streamcontext;

void Audio_StreamIRQ_SPU(void)
{
	//Disable SPU IRQ until we've finished streaming more data
	SPU_CTRL &= ~0x0040;
	
	//Swap active buffer
	audio_streamcontext.db_active ^= 1;
	audio_streamcontext.spu_pos = 0;
	
	//Update addresses
	audio_streamcontext.spu_addr = BUFFER_START_ADDR + CHUNK_SIZE * audio_streamcontext.db_active;
	
	SPU_IRQ_ADDR = SPU_RAM_ADDR(audio_streamcontext.spu_addr);
	SPU_CHANNELS[0].loop_addr = SPU_RAM_ADDR(audio_streamcontext.spu_addr);
	SPU_CHANNELS[1].loop_addr = SPU_RAM_ADDR(audio_streamcontext.spu_addr + BUFFER_SIZE);
	
	//Unmute the channels
	SPU_CHANNELS[0].vol_left  = 0x3FFF;
	SPU_CHANNELS[1].vol_right = 0x3FFF;
	
	//Continue streaming from CD
	CdlLOC pos;
	CdIntToPos(audio_streamcontext.lba + audio_streamcontext.pos, &pos);
	CdControlF(CdlReadN, (u8*)&pos);
}

void Audio_StreamIRQ_CD(u8 event, u8 *payload)
{
	(void)payload;
	
	//Ignore all events other than a sector being ready
	if (event != CdlDataReady)
		return;
	
	//Fetch the sector that has been read from the drive
	u8 sector[2048];
	CdGetSector(sector, 2048 / 4);
	audio_streamcontext.pos++;
	
	//Write sector to SPU RAM
	SpuSetTransferStartAddr(audio_streamcontext.spu_addr + audio_streamcontext.spu_pos);
	SpuWrite(sector, 2048);
	audio_streamcontext.spu_pos += 2048;
	
	//Start SPU IRQ if finished reading
	if (audio_streamcontext.spu_pos >= CHUNK_SIZE)
	{
		CdControlF(CdlPause, NULL);
		SPU_CTRL |= 0x0040;
	}
}

//Audio interface
void Audio_Init(void)
{
	//Initialize SPU
	SpuInit();
}

void Audio_Quit(void)
{
	
}

void Audio_Test(void)
{
	//Find file
	CdlFILE file;
	if (!CdSearchFile(&file, "\\MENU\\MENU.MUS;1"))
	{
		printf("fail\n");
		return;
	}
	
	//Set IRQs
	CdReadyCallback(Audio_StreamIRQ_CD);
	
	EnterCriticalSection();
	InterruptCallback(9, Audio_StreamIRQ_SPU);
	ExitCriticalSection();
	
	//Initialize context
	audio_streamcontext.lba = CdPosToInt(&file.pos);
	audio_streamcontext.length = file.size >> 11;
	audio_streamcontext.pos = 0;
	
	//Preload chunk
	audio_streamcontext.db_active = 1;
	Audio_StreamIRQ_SPU();
	
	while (audio_streamcontext.spu_pos < CHUNK_SIZE)
		__asm__("nop");
	
	// Start playing on SPU channels 0 and 1.
	for (int i = 0; i < 2; i++)
	{
		SPU_CHANNELS[i].vol_left   = 0x0000;
		SPU_CHANNELS[i].vol_right  = 0x0000;
		SPU_CHANNELS[i].addr       = SPU_RAM_ADDR(BUFFER_START_ADDR + BUFFER_SIZE * i);
		SPU_CHANNELS[i].freq       = SAMPLE_RATE;
		SPU_CHANNELS[i].adsr_param = 0xdfee80ff; // 0xdff18087
	}
	SPU_KEY_ON = 0x0003;
	
	Audio_StreamIRQ_SPU();
}
