/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../gfx.h"

#include "gl.h"
#include "../main.h"

#include "SDL.h"

//GFX constants
#define WINDOW_SCALE 3
#define WINDOW_WIDTH  (SCREEN_WIDTH * WINDOW_SCALE)
#define WINDOW_HEIGHT (SCREEN_HEIGHT * WINDOW_SCALE)

//Window
SDL_Window *window;
SDL_GLContext gl_context;

boolean window_closed;

//Gfx functions
void Gfx_Init(void)
{
	//Set window hints
	#if PSXF_GL == PSXF_GL_MODERN
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
		SDL_GL_SetAttribute(GLFW_CONTEXT_VERSION_MAJOR, 3);
		SDL_GL_SetAttribute(GLFW_CONTEXT_VERSION_MINOR, 2);
	#elif PSXF_GL == PSXF_GL_LEGACY
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	#elif PSXF_GL == PSXF_GL_ES
		#ifndef PSXF_EMSCRIPTEN
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		#endif
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	#endif
	
	//Create window and context
	if ((window = SDL_CreateWindow("PSXFunkin", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)) == NULL)
	{
		sprintf(error_msg, "[Gfx_Init] Failed to create window: %s", SDL_GetError());
		ErrorLock();
	}
	if ((gl_context = SDL_GL_CreateContext(window)) == NULL)
	{
		sprintf(error_msg, "[Gfx_Init] Failed to create OpenGL context: %s", SDL_GetError());
		ErrorLock();
	}
	
	//Use swap interval
	if (SDL_GL_SetSwapInterval(-1) < 0)
		SDL_GL_SetSwapInterval(1);
	
	//Initialize GL
	GL_Init();
}

void Gfx_Quit(void)
{
	//Deinitialize GL
	GL_Quit();
	
	//Destroy window and context
	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
}

void Gfx_Flip(void)
{
	//FPS counter
	static int fps_i = 0;
	static Uint32 fps_t = 0;
	
	if (SDL_GetTicks() >= fps_t)
	{
		char buf[0x80];
		sprintf(buf, "PSXFunkin [%d fps]", fps_i);
		SDL_SetWindowTitle(window, buf);
		fps_t = SDL_GetTicks() + 1000;
		fps_i = 0;
	}
	fps_i++;
	
	//GL flip
	GL_Flip();
	
	//SDL2 flip
	SDL_GL_SwapWindow(window);
	
	//Process events
	SDL_Event event;
	
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
			{
				window_closed = true;
				break;
			}
			default:
				break;
		}
	}
}
