/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_AUDIO_H
#define PSXF_GUARD_AUDIO_H

#include "psx.h"

//Audio interface
void Audio_Init(void);
void Audio_Quit(void);
void Audio_PlayMusFile(CdlFILE *file);
void Audio_PlayMus(const char *path);
void Audio_StopMus(void);

#endif
