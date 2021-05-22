#ifndef _MEM_H
#define _MEM_H

#include "psx.h"

u8 Mem_Init(void *ptr, size_t size);
void *Mem_Alloc(size_t size);
void Mem_Free(void *ptr);
void Mem_GetStat(size_t *used, size_t *size, size_t *max);

#endif
