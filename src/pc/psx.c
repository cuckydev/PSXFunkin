/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../psx.h"

#include "../main.h"

#ifdef PSXF_EMSCRIPTEN
	#include "SDL.h"
	#include "SDL_hints.h"
	
	//Window closed
	extern boolean window_closed;
#else
	#define GLFW_INCLUDE_NONE
	#include <GLFW/glfw3.h>
	
	//Window
	extern GLFWwindow *window;
#endif

//Arguments
int my_argc;
char **my_argv;

//PSX functions
void PSX_Init(void)
{
	#ifdef PSXF_EMSCRIPTEN
		//Initialize SDL2
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
		{
			sprintf(error_msg, "[PSX_Init] Failed to initialize SDL2");
			ErrorLock();
		}
	#else
		//Initialize GLFW
		if (glfwInit() != GLFW_TRUE)
		{
			sprintf(error_msg, "[PSX_Init] Failed to initialize GLFW");
			ErrorLock();
		}
	#endif
}

void PSX_Quit(void)
{
	#ifdef PSXF_EMSCRIPTEN
		//Quit SDL2
		SDL_Quit();
	#else
		//Quit GLFW
		glfwTerminate();
	#endif
}

boolean PSX_Running(void)
{
	#ifdef PSXF_EMSCRIPTEN
		return window_closed;
	#else
		return !glfwWindowShouldClose(window);
	#endif
}

//Misc. functions
#include <stdarg.h>

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
	vprintf(format, vl);
	va_end(vl);
}
