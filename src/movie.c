/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "movie.h"

#include "psx.h"
#include "main.h"

#include "strplay.c"

//Movie interface
void Movie_Play(const char *path, unsigned long length)
{
	STRFILE sfile;
	strcpy(sfile.FileName, path);
	sfile.Xres = 320;
	sfile.Yres = 240;
	sfile.NumFrames = length;
	if (PlayStr(320, 240, 0, 0, &sfile) == 0)
	{
		sprintf(error_msg, "[Movie_Play] Failed to play \"%s\"", path);
		ErrorLock();
	}
}