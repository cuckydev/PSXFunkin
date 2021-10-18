/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../gfx.h"

#include "gl.h"
#include "../main.h"

#if PSXF_GL == PSXF_GL_ES
	#include <GLES2/gl2.h>
#else
	#include "glad/glad.h"
#endif
#include <GLFW/glfw3.h>

//GFX constants
#define WINDOW_SCALE 3
#define WINDOW_WIDTH  (SCREEN_WIDTH * WINDOW_SCALE)
#define WINDOW_HEIGHT (SCREEN_HEIGHT * WINDOW_SCALE)

//Window
GLFWwindow *window;

//Internal gfx functions
static void Gfx_FramebufferSizeCallback(GLFWwindow *window, int fb_width, int fb_height)
{
	(void)window;
	
	//Center the viewport within the window while maintaining the aspect ratio
	GLfloat viewport_width, viewport_height;
	if ((float)fb_width / (float)fb_height > (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)
	{
		viewport_width = fb_height * SCREEN_WIDTH / SCREEN_HEIGHT;
		viewport_height = fb_height;
	}
	else
	{
		viewport_width = fb_width;
		viewport_height = fb_width * SCREEN_HEIGHT / SCREEN_WIDTH;
	}
	
	glViewport((fb_width - viewport_width) / 2, (fb_height - viewport_height) / 2, viewport_width, viewport_height);
}

//Gfx functions
void Gfx_Init(void)
{
	//Set window hints
#if PSXF_GL == PSXF_GL_MODERN
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#elif PSXF_GL == PSXF_GL_LEGACY
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#elif PSXF_GL == PSXF_GL_ES
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	
	//Get monitor video mode
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	
	const GLFWvidmode *mode;
	if (monitor != NULL)
		mode = glfwGetVideoMode(monitor);
	else
		mode = NULL;
	
	//Create window
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PSXFunkin", NULL, NULL);
	if (!window)
	{
		sprintf(error_msg, "[Gfx_Init] Failed to create GLFW window");
		ErrorLock();
	}
	glfwMakeContextCurrent(window);
	
	//Center window
	if (mode != NULL)
		glfwSetWindowPos(window, (mode->width - WINDOW_WIDTH) / 2, (mode->height - WINDOW_HEIGHT) / 2);
	glfwShowWindow(window);
	
	//Define callback for window resizing
	glfwSetFramebufferSizeCallback(window, Gfx_FramebufferSizeCallback);
	
	//Enable vsync
	if (glfwExtensionSupported("GLX_EXT_swap_control_tear") || glfwExtensionSupported("WGL_EXT_swap_control_tear"))
		glfwSwapInterval(-1);
	else
		glfwSwapInterval(1);
	
	#if PSXF_GL != PSXF_GL_ES
		//Initialize GLAD
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			sprintf(error_msg, "[Gfx_Init] Failed to initialize GLAD");
			ErrorLock();
		}
		
		#if PSXF_GL == PSXF_GL_MODERN
			if (!GLAD_GL_VERSION_3_2)
			{
				sprintf(error_msg, "[Gfx_Init] OpenGL 3.2 is not supported");
				ErrorLock();
			}
		#elif PSXF_GL == PSXF_GL_LEGACY
			if (!GLAD_GL_VERSION_2_1)
			{
				sprintf(error_msg, "[Gfx_Init] OpenGL 2.1 is not supported");
				ErrorLock();
			}
		#endif
	#endif
	
	//Initialize GL
	GL_Init();
}

void Gfx_Quit(void)
{
	//Deinitialize GL
	GL_Quit();
	
	//Destroy window
	glfwDestroyWindow(window);
}

void Gfx_Flip(void)
{
	//FPS counter
	static int fps_i = 0;
	static double fps_t = 0.0;
	
	if (glfwGetTime() >= fps_t)
	{
		char buf[0x80];
		sprintf(buf, "PSXFunkin [%d fps]", fps_i);
		glfwSetWindowTitle(window, buf);
		fps_t = glfwGetTime() + 1.0;
		fps_i = 0;
	}
	fps_i++;
	
	//GL flip
	GL_Flip();
	
	//GLFW flip
	glfwSwapBuffers(window);
	glfwPollEvents();
}
