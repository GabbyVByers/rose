
/*
 *   Source File [rose_peripherals.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

bool ROSE_MousePressing(uint32_t button)
{
	return (curr_mouse_state & SDL_BUTTON_MASK(button)) != 0;
}

bool ROSE_MousePressed(uint32_t button)
{
	return ((prev_mouse_state & SDL_BUTTON_MASK(button)) == 0)
		&& ((curr_mouse_state & SDL_BUTTON_MASK(button)) != 0);
}

bool ROSE_MouseReleased(uint32_t button)
{
	return ((prev_mouse_state & SDL_BUTTON_MASK(button)) != 0)
		&& ((curr_mouse_state & SDL_BUTTON_MASK(button)) == 0);
}

void ROSE_MouseHide(void)
{
	saved_mouse_px = mouse_xpos;
	saved_mouse_py = mouse_ypos;
	SDL_SetWindowRelativeMouseMode(window, true);
}

void ROSE_MouseReveal(void)
{
	mouse_xpos = saved_mouse_px;
	mouse_ypos = saved_mouse_py;
	SDL_WarpMouseInWindow(window, mouse_xpos, mouse_ypos);
	SDL_SetWindowRelativeMouseMode(window, false);
}

bool ROSE_KeyboardPressing(ROSE_Scancode button)
{
	return curr_keyboard_state[button];
}

bool ROSE_KeyboardPressed(ROSE_Scancode button)
{
	return (!prev_keyboard_state[button])
		&& (curr_keyboard_state[button]);
}

bool ROSE_KeyboardReleased(ROSE_Scancode button)
{
	return (prev_keyboard_state[button])
		&& (!curr_keyboard_state[button]);
}

