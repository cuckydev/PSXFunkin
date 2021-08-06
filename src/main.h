#ifndef _MAIN_H
#define _MAIN_H

//Game loop
typedef enum
{
	GameLoop_Menu,
	GameLoop_Stage,
} GameLoop;

extern GameLoop gameloop;

//Error handler
extern char error_msg[0x200];
void ErrorLock();

#endif
