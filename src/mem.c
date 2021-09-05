/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mem.h"

#ifndef PSXF_STDMEM

#include "main.h"
#include "gfx.h"
#include "psx.h"
#include "random.h"

typedef struct
{
	void *prev, *next;
	size_t size;
	u8 i;
} Mem_Header;
#define MEM_HEDSIZE (MEM_ALIGN(sizeof(Mem_Header)))

static Mem_Header *mem = NULL;
static size_t mem_used, mem_size, mem_max;

#ifdef MEM_LEAK_CHECK
	static const char *signs[256];
#endif

u8 Mem_Init(void *ptr, size_t size)
{
	//Make sure there's enough space for mem header
	if (size < MEM_HEDSIZE)
		return 1;
	
	//Get mem pointer (16 byte alignment)
	mem = (Mem_Header*)MEM_ALIGN(ptr);
	mem_used = MEM_HEDSIZE;
	mem_size = size - ((size_t)mem - (size_t)ptr);
	mem_max = mem_used;
	
	//Initial mem header
	mem->prev = NULL;
	mem->next = NULL;
	mem->size = mem_size - MEM_HEDSIZE;
	
	#ifdef MEM_LEAK_CHECK
		memset(signs, 0, sizeof(signs));
	#endif
	
	return 0;
}

static Mem_Header *Mem_GetHeader(void *ptr)
{
	if (ptr == NULL)
		return NULL;
	return (Mem_Header*)((size_t)ptr - MEM_HEDSIZE);
}

#ifdef MEM_LEAK_CHECK
void *Mem_Alloc2(const char *sign, size_t size)
#else
void *Mem_Alloc(size_t size)
#endif
{
	size_t header_size;
	size_t next_pos;
	Mem_Header *new_header;
	void *new_block;
	
	//Get block to allocate in front of
	Mem_Header *header = mem;
	if (header == NULL)
		return NULL;
	
	header_size = 0;
	next_pos = MEM_ALIGN((size_t)header + MEM_HEDSIZE);
	
	while (header != NULL)
	{
		//If there's no block to check up next, compare against the end of the heap
		if (header->next == NULL)
		{
			//Check if there's enough space to allocate
			if ((next_pos + MEM_HEDSIZE + size) > 
				((size_t)mem + MEM_HEDSIZE + mem->size))
				return NULL;
			break;
		}
		
		//Check if there's enough space to allocate
		if (((size_t)header->next - MEM_HEDSIZE) >= (next_pos + MEM_HEDSIZE + size))
			break;
		
		//Check next block
		if ((header = Mem_GetHeader(header->next)) == NULL)
			return NULL;
		header_size = header->size;
		next_pos = MEM_ALIGN((size_t)header + MEM_HEDSIZE + header_size);
	}
	
	//Setup header
	new_header = (Mem_Header*)next_pos;
	new_header->size = size;
	
	new_block = (void*)(next_pos + MEM_HEDSIZE);
	
	//Link header to previous and next headers
	new_header->prev = (void*)((size_t)header + MEM_HEDSIZE);
	new_header->next = header->next;
	
	//Link next header to us
	if (header->next != NULL)
		Mem_GetHeader(header->next)->prev = new_block;
	
	//Link previous header to us
	header->next = new_block;
	
	mem_used += new_header->size + MEM_HEDSIZE;
	if (mem_used > mem_max)
		mem_max = mem_used;
	
	#ifdef MEM_LEAK_CHECK
		for (int i = 0; i < COUNT_OF(signs); i++)
		{
			if (signs[i] == NULL)
			{
				signs[i] = sign;
				new_header->i = i;
				break;
			}
		}
	#endif
	
	return new_block;
}

void Mem_Free(void *ptr)
{
	Mem_Header *header, *header2;
	
	//Get this block's header
	header = Mem_GetHeader(ptr);
	if (header == NULL)
		return;
	
	#ifdef MEM_LEAK_CHECK
		signs[header->i] = NULL;
	#endif
	
	//Unlink from previous block
	header2 = Mem_GetHeader(header->prev);
	if (header2 != NULL)
		header2->next = header->next;
	
	//Unlink from next block
	header2 = Mem_GetHeader(header->next);
	if (header2 != NULL)
		header2->prev = header->prev;
	
	mem_used -= header->size + MEM_HEDSIZE;
}

#ifdef MEM_STAT
	void Mem_GetStat(size_t *used, size_t *size, size_t *max)
	{
		if (used != NULL)
			*used = mem_used;
		if (size != NULL)
			*size = mem_size;
		if (max != NULL)
			*max = mem_max;
		
		#ifdef MEM_LEAK_CHECK
			for (int i = 0; i < COUNT_OF(signs); i++)
				if (signs[i] != NULL)
					FntPrint("%s\n", signs[i]);
		#endif
		
		#ifdef MEM_BAR
			#define BAR_WIDTH 256
			#define BAR_LEFT ((SCREEN_WIDTH - BAR_WIDTH) / 2)
			#define BAR_ADDRTOP(addr) ((addr) * BAR_WIDTH / mem_size)
			
			RECT bar = {BAR_LEFT, 12, 0, 4};
			
			Mem_Header *header = mem;
			size_t header_size = 0;
			
			u32 oseed = RandomGetSeed();
			RandomSeed(1394);
			
			while (header != NULL)
			{
				size_t addr = (size_t)header - (size_t)mem;
				int left = BAR_ADDRTOP(addr);
				int right = BAR_ADDRTOP(addr + MEM_HEDSIZE + header_size);
				
				bar.x = BAR_LEFT + left;
				bar.w = right - left;
				Gfx_DrawRect(&bar, Random8(), Random8(), Random8());
				
				if ((header = Mem_GetHeader(header->next)) != NULL)
					header_size = header->size;
			}
			
			bar.x = BAR_LEFT;
			bar.w = BAR_WIDTH;
			Gfx_DrawRect(&bar, 32, 32, 32);
			
			RandomSeed(oseed);
		#endif
	}
#endif

#endif
