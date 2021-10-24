/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_PSX_H
#define PSXF_GUARD_PSX_H

extern int my_argc;
extern char **my_argv;

#ifdef PSXF_PC
	//Headers
	#include <sys/types.h>
	#include <stdio.h>
	#include <stdlib.h>
	
	#include <stdint.h>
	#include <stddef.h>
	#include <string.h>
	#include <time.h>
	
	//Fixed size types
	typedef uint8_t  u8;
	typedef int8_t   s8;
	typedef uint16_t u16;
	typedef int16_t  s16;
	typedef uint32_t u32;
	typedef int32_t  s32;
	typedef uint64_t u64;
	typedef int64_t  s64;
	
	//CD types
	typedef struct {
		s32 x, y, w, h;
	} RECT;
	
	typedef struct {
		char path[32];
	} CdlFILE;
	
	//Misc. functions
	void FntPrint(const char *format, ...);
	void MsgPrint(const char *format, ...);
#else
	//Headers
	#include <sys/types.h>
	#include <stdio.h>

	#include <libetc.h>
	#include <libgte.h>
	#include <libgpu.h>
	#include <libspu.h>
	#include <libcd.h>
	#include <libsnd.h>
	#include <libapi.h>

	#include <stddef.h>
	#include <string.h>

	//Fixed size types
	typedef u_char             u8;
	typedef signed char        s8;
	typedef u_short            u16;
	typedef signed short       s16;
	typedef u_long             u32;
	typedef signed int         s32;
	typedef unsigned long long u64;
	typedef signed long long   s64;
	
	//Misc. functions
	#define MsgPrint FntPrint
#endif

//Boolean type
typedef s8 boolean;
#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif

//Point type
typedef struct
{
	short x, y;
} POINT;

//Common macros
#define sizeof_member(type, member) sizeof(((type *)0)->member)

#define COUNT_OF(x) (sizeof(x) / sizeof(0[x]))
#define COUNT_OF_MEMBER(type, member) (sizeof_member(type, member) / sizeof_member(type, member[0]))

#define TYPE_SIGNMIN(utype, stype) ((stype)((((utype)-1) >> 1) + 1))

//PSX functions
void PSX_Init(void);
void PSX_Quit(void);
boolean PSX_Running(void);

#endif
