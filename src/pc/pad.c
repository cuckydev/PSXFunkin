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
Pad pad_state;

//Pad functions
void Pad_Init(void)
{
	pad_state.held = pad_state.press = 0;
	pad_state.left_x = pad_state.left_y = pad_state.right_x = pad_state.right_y = 0;
}

void Pad_Quit(void)
{
	
}

void Pad_Update(void)
{
	//Get next held state
	u16 next_held = 0;
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
}
