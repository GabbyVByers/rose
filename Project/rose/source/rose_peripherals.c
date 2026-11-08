
/*
 *   Source [rose_peripherals.c]
 */

#include "rose.h"

bool ROSE_MousePressing(uInt32 mouse_button)
{
	return (rose__curr_mouse_state & SDL_BUTTON_MASK(mouse_button)) != 0;
}

bool ROSE_MousePressed(uInt32 mouse_button)
{
	return ((rose__prev_mouse_state & SDL_BUTTON_MASK(mouse_button)) == 0)
		&& ((rose__curr_mouse_state & SDL_BUTTON_MASK(mouse_button)) != 0);
}

bool ROSE_MouseReleased(uInt32 mouse_button)
{
	return ((rose__prev_mouse_state & SDL_BUTTON_MASK(mouse_button)) != 0)
		&& ((rose__curr_mouse_state & SDL_BUTTON_MASK(mouse_button)) == 0);
}

void ROSE_MouseHide(void)
{
	rose__saved_mouse_px = rose_mouse.xpos;
	rose__saved_mouse_py = rose_mouse.ypos;
	SDL_SetWindowRelativeMouseMode(rose__window, true);
}

void ROSE_MouseReveal(void)
{
	rose_mouse.xpos = rose__saved_mouse_px;
	rose_mouse.ypos = rose__saved_mouse_py;
	SDL_WarpMouseInWindow(rose__window, rose_mouse.xpos, rose_mouse.ypos);
	SDL_SetWindowRelativeMouseMode(rose__window, false);
}

bool ROSE_KeyboardPressing(ROSE_Scancode scancode)
{
	assert(scancode < ROSE_SCANCODE_COUNT);
	return rose__curr_keyboard_state[scancode];
}

bool ROSE_KeyboardPressed(ROSE_Scancode scancode)
{
	assert(scancode < ROSE_SCANCODE_COUNT);
	return (!rose__prev_keyboard_state[scancode])
		&& (rose__curr_keyboard_state[scancode]);
}

bool ROSE_KeyboardReleased(ROSE_Scancode scancode)
{
	assert(scancode < ROSE_SCANCODE_COUNT);
	return (rose__prev_keyboard_state[scancode])
		&& (!rose__curr_keyboard_state[scancode]);
}

