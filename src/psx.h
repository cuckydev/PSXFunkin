#ifndef _PSX_H
#define _PSX_H

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


#endif
