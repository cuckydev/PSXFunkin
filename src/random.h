#ifndef _RANDOM_H
#define _RANDOM_H

#include "psx.h"

//Random functions
void RandomSeed(u32 seed);
u32 RandomGetSeed();
u8 Random8();
u16 Random16();
u32 Random32();
s32 RandomRange(s32 x, s32 y);

#endif
