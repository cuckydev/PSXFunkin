/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _MEM_H
#define _MEM_H

#include "psx.h"

#define MEM_ALIGN(x) (((size_t)x + 0xF) & ~0xF)

#ifdef PSXF_STDMEM

#define Mem_Init(a, b)
#define Mem_Alloc malloc
#define Mem_Free free

#else

//#define MEM_STAT
//#define MEM_BAR //MEM_STAT must be defined for display
//#define MEM_LEAK_CHECK //MEM_STAT must be defined for display

u8 Mem_Init(void *ptr, size_t size);
#ifdef MEM_LEAK_CHECK
	#define STRINGIZE(x) #x
	#define TOSTRING(x) STRINGIZE(x)
	#define AT __FILE__ ":" TOSTRING(__LINE__)
	
	void *Mem_Alloc2(const char *sign, size_t size);
	#define Mem_Alloc(size) Mem_Alloc2(AT, size)
#else
	void *Mem_Alloc(size_t size);
	#define Mem_Alloc2(sign, size) Mem_Alloc(size)
#endif
void Mem_Free(void *ptr);
#ifdef MEM_STAT
	void Mem_GetStat(size_t *used, size_t *size, size_t *max);
#endif

#endif

#endif
