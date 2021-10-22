/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "../pad.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

//Window
extern GLFWwindow *window;

//Pad state
Pad pad_state, pad_state_2;

#ifdef PSXF_NETWORK

//Stupid type code
char pad_type[PAD_TYPE_CHARS + 1];
boolean pad_backspace, pad_backspace_held;

static char pad_type_internal[PAD_TYPE_CHARS + 1];
void Pad_TypeCallback(GLFWwindow *proc_window, unsigned int code)
{
	if (proc_window != window || (code & ~0x7F) || !(code & 0x60))
		return;
	size_t i = strlen(pad_type_internal);
	if (i == PAD_TYPE_CHARS)
		return;
	pad_type_internal[i] = code;
	pad_type_internal[i+1] = '\0';
}

#endif

//Pad functions
void Pad_Init(void)
{
	//Clear pad states
	pad_state.held = pad_state.press = 0;
	pad_state.left_x = pad_state.left_y = pad_state.right_x = pad_state.right_y = 0;
	
	pad_state_2.held = pad_state_2.press = 0;
	pad_state_2.left_x = pad_state_2.left_y = pad_state_2.right_x = pad_state_2.right_y = 0;
	
	#ifdef PSXF_NETWORK
		//Set up typing
		pad_type[0] = pad_type_internal[0] = '\0';
		glfwSetCharCallback(window, Pad_TypeCallback);
		pad_backspace = pad_backspace_held = 0;
	#endif
}

void Pad_Quit(void)
{
	
}

void Pad_Update(void)
{
	u16 next_held;
	
	//Get next held state
	next_held = 0;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		next_held |= PAD_SQUARE;
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		next_held |= PAD_CROSS;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		next_held |= PAD_TRIANGLE;
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		next_held |= PAD_CIRCLE;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		next_held |= PAD_LEFT;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		next_held |= PAD_UP;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		next_held |= PAD_RIGHT;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		next_held |= PAD_DOWN;
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
		next_held |= PAD_START;
	
	//Update pad state
	pad_state.press = next_held & ~pad_state.held;
	pad_state.held = next_held;
	
	//Get next held state 2
	next_held = 0;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		next_held |= PAD_SQUARE;
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		next_held |= PAD_CROSS;
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		next_held |= PAD_TRIANGLE;
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		next_held |= PAD_CIRCLE;
	
	//Update pad state 2
	pad_state_2.press = next_held & ~pad_state_2.held;
	pad_state_2.held = next_held;
	
	#ifdef PSXF_NETWORK
		//Update type state
		strcpy(pad_type, pad_type_internal);
		pad_type_internal[0] = '\0';
		
		boolean backspace_held = glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS;
		pad_backspace = backspace_held & ~pad_backspace_held;
		pad_backspace_held = backspace_held;
	#endif
}
