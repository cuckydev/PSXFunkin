/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "random.h"

//Random functions
static u32 rand_seed;

void RandomSeed(u32 seed)
{
	rand_seed = seed;
}

u32 RandomGetSeed(void)
{
	return rand_seed;
}

u8 Random8(void)
{
	return Random16() >> 4;
}

u16 Random16(void)
{
	rand_seed = rand_seed * 214013L + 2531011L;
	return rand_seed >> 16;
}

u32 Random32(void)
{
	return ((u32)Random16() << 16) | Random16();
}

s32 RandomRange(s32 x, s32 y)
{
	return x + Random16() % ((s32)y - (s32)x + 1);
}
