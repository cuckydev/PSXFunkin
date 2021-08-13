#include "../psx.h"

#include "../main.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

//Arguments
int my_argc;
char **my_argv;

//PSX functions
void PSX_Init(void)
{
	//Initialize GLFW
	if (glfwInit() != GL_TRUE)
	{
		sprintf(error_msg, "[PSX_Init] Failed to initialize GLFW");
		ErrorLock();
	}
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
