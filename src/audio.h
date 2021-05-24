#ifndef _AUDIO_H
#define _AUDIO_H

#include "psx.h"

//XA enumerations
typedef enum
{
	XA_Menu,   //MENU.XA
	XA_Week1A, //WEEK1A.XA
	XA_Week1B, //WEEK1B.XA
	XA_Week3A, //WEEK3A.XA
	XA_Week3B, //WEEK3B.XA
	XA_Week4A, //WEEK4A.XA
	XA_Week4B, //WEEK4B.XA
	
	XA_Max,
} XA_File;

typedef enum
{
	//MENU.XA
	XA_GettinFreaky, //Gettin' Freaky
	XA_GameOver,     //Game Over
	//WEEK1A.XA
	XA_Bopeebo, //Bopeebo
	XA_Fresh,   //Fresh
	//WEEK1B.XA
	XA_Dadbattle, //DadBattle
	//WEEK3A.XA
	XA_Pico,   //Pico
	XA_Philly, //Philly
	//WEEK3B.XA
	XA_Blammed, //Blammed
	//WEEK4A.XA
	XA_SatinPanties, //Satin Panties
	XA_High,         //High
	//WEEK4B.XA
	XA_MILF, //M.I.L.F
	XA_Test, //Test
} XA_Track;

//Audio functions
void Audio_Init();
void Audio_GetXAFile(CdlFILE *file, XA_Track track);
void Audio_PlayXA_File(CdlFILE *file, u8 volume, u8 channel, boolean loop);
void Audio_PlayXA_Track(XA_Track track, u8 volume, u8 channel, boolean loop);
void Audio_PlayXA(const char *path, u8 volume, u8 channel, boolean loop);
void Audio_PauseXA();
void Audio_StopXA();
void Audio_ChannelXA(u8 channel);
s32 Audio_TellXA_Sector();
s32 Audio_TellXA_Milli();
boolean Audio_PlayingXA();
void Audio_ProcessXA();

#endif
