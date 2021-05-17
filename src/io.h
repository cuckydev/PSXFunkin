#ifndef _IO_H
#define _IO_H

#include "psx.h"

typedef u32* IO_Data;

//IO constants
#define IO_SECT_SIZE 2048

//IO functions
void IO_Init();
IO_Data IO_Read(const char *path);

#endif
