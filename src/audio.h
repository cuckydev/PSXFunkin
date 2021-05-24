#ifndef _AUDIO_H
#define _AUDIO_H

#include "psx.h"

//Audio functions
void Audio_Init();
void Audio_PlayXA_File(CdlFILE *file, u8 volume, u8 channel, boolean loop);
void Audio_PlayXA(const char *path, u8 volume, u8 channel, boolean loop);
void Audio_PauseXA();
void Audio_StopXA();
void Audio_ChannelXA(u8 channel);
s32 Audio_TellXA_Sector();
s32 Audio_TellXA_Milli();
boolean Audio_PlayingXA();
void Audio_ProcessXA();

#endif
