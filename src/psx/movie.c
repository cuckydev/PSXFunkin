/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../movie.h"

#include "strplay.c"

//Movie interface
void Movie_Play(const char *path, u32 length)
{
	STRFILE file;
	strcpy(file.FileName, path);
	file.Xres = 320;
	file.Yres = 240;
	file.NumFrames = length;
	PlayStr(320, 240, 0, 0, &file);
}
