#include "../psx.h"

#include "../main.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

//Window
extern GLFWwindow *window;

//Arguments
int my_argc;
char **my_argv;

//PSX functions
void PSX_Init(void)
{
	//Initialize GLFW
	if (glfwInit() != GLFW_TRUE)
	{
		sprintf(error_msg, "[PSX_Init] Failed to initialize GLFW");
		ErrorLock();
	}
}

void PSX_Quit(void)
{
	//Quit GLEW
	glfwTerminate();
}

boolean PSX_Running(void)
{
	return !glfwWindowShouldClose(window);
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
