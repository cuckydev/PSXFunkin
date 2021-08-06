#ifndef _FIXED_H
#define _FIXED_H

#include "psx.h"

//Fixed types and constants
typedef s32 fixed_t;

typedef struct
{
	fixed_t x, y, w, h;
} RECT_FIXED;

typedef struct
{
	fixed_t x, y;
} POINT_FIXED;

#define FIXED_SHIFT (10)
#define FIXED_UNIT  (1 << FIXED_SHIFT)
#define FIXED_LAND  (FIXED_UNIT - 1)
#define FIXED_UAND  (~FIXED_LAND)

#define FIXED_DEC(d, f) (((fixed_t)(d) << FIXED_SHIFT) / (f))

#define FIXED_MUL(x, y) (((s64)(x) * (y)) >> FIXED_SHIFT)
#define FIXED_DIV(x, y) (((s32)(x) << FIXED_SHIFT) / (y))

#endif
