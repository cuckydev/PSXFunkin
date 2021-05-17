#ifndef _AUDIO_H
#define _AUDIO_H

//void PrepareXA(void);
//void PlayXA(int startp, int endp, int index, int addon);
//int PlayingXA(void);
//void UnprepareXA(void);

//Audio functions
void Audio_Init();
void Audio_PlayXA_Pos(int start, int end, int volume, int channel, int loop);
void Audio_PlayXA(const char *path, int volume, int channel, int loop);
void Audio_ChannelXA(int channel);
int Audio_TellXA_Sector();
int Audio_TellXA_Milli();
int Audio_PlayingXA();
void Audio_StopXA();

#endif
