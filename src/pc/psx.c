#include "../psx.h"

#include "../main.h"

#include "SDL.h"

//Arguments
int my_argc;
char **my_argv;

//PSX functions
void PSX_Init(void)
{
	//Initialize SDL2
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0)
	{
		sprintf(error_msg, "Failed to initialize SDL2: %s", SDL_GetError());
		ErrorLock();
	}
}

//Format alloc
#include <stdarg.h>
#include <stdlib.h>

char *FormatAlloc_VAList(const char *format, va_list vl)
{
	va_list vlc;
	
	//Get formatted string's size
	va_copy(vlc, vl);
	int size = vsnprintf(NULL, 0, format, vlc);
	va_end(vlc);
	
	//Make sure size result is valid
	if (size < 0)
		return NULL;
	size++;
	
	//Allocate buffer based off of retrieved size
	char *string = malloc(size);
	if (string == NULL)
		return NULL;
	
	//Construct formatted string
	va_copy(vlc, vl);
	vsprintf(string, format, vlc); //vsnprintf(string, size, format, vlc);
	va_end(vlc);
	
	//Return formatted string
	return string;
}

//Misc. functions
void FntPrint(const char *format, ...)
{
	//Get message
	va_list vl;
	va_start(vl, format);
	vprintf(format, vl);
	va_end(vl);
}

void MsgPrint(const char *format, ...)
{
	//Get message
	va_list vl;
	va_start(vl, format);
	char *msg = FormatAlloc_VAList(format, vl);
	va_end(vl);
	
	//Display message
	SDL_ShowSimpleMessageBox(0, "FntPrint", msg, NULL);
	free(msg);
}