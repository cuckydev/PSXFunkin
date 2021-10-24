/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_MEM_H
#define PSXF_GUARD_MEM_H

#include "psx.h"

#define MEM_ALIGNSIZE 0x10
#define MEM_ALIGN(x) (((size_t)(x) + 0xF) & ~0xF)

#ifdef PSXF_STDMEM

#define Mem_Init(a, b)
#define Mem_Alloc malloc
#define Mem_Free free

#else

//#define MEM_STAT

u8 Mem_Init(void *ptr, size_t size);
void *Mem_Alloc(size_t size);
void Mem_Free(void *ptr);
#ifdef MEM_STAT
	void Mem_GetStat(size_t *used, size_t *size, size_t *max);
#endif

#endif

#endif
