#include "random.h"

//Random functions
static u32 rand_seed;

void RandomSeed(u32 seed)
{
	rand_seed = seed;
}

u16 Random16()
{
	rand_seed = rand_seed * 214013L + 2531011L;
	return rand_seed >> 16;
}

u32 Random32()
{
	return ((u32)Random16() << 16) | Random16();
}

s32 RandomRange(s32 x, s32 y)
{
	return x + Random16() % ((s32)y - (s32)x + 1);
}
