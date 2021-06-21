#ifndef _MUTIL_H
#define _MUTIL_H

#include "psx.h"

//Math utility functions
s16 MUtil_Sin(u8 x);
s16 MUtil_Cos(u8 x);
void MUtil_RotatePoint(POINT *p, s16 s, s16 c);

#endif
