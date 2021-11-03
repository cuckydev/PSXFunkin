/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_RANDOM_H
#define PSXF_GUARD_RANDOM_H

#include "psx.h"

//Random functions
void RandomSeed(u32 seed);
u32 RandomGetSeed();
u8 Random8();
u16 Random16();
u32 Random32();
s32 RandomRange(s32 x, s32 y);

#endif
