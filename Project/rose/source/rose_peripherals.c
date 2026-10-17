
/*
 *   Source File [rose_peripherals.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

bool ROSE_MousePressing(u32 button)
{
	assert(button != 0);
	return (curr_mouse_state & SDL_BUTTON_MASK(button)) != 0;
}

bool ROSE_MousePressed(u32 button)
{
	assert(button != 0);
	return ((prev_mouse_state & SDL_BUTTON_MASK(button)) == 0)
		&& ((curr_mouse_state & SDL_BUTTON_MASK(button)) != 0);
}

bool ROSE_MouseReleased(u32 button)
{
	assert(button != 0);
	return ((prev_mouse_state & SDL_BUTTON_MASK(button)) != 0)
		&& ((curr_mouse_state & SDL_BUTTON_MASK(button)) == 0);
}

void ROSE_MouseHide(void)
{
	saved_mouse_px = mouse_xpos;
	saved_mouse_py = mouse_ypox;

	if (!SDL_SetWindowRelativeMouseMode(window, true))
	{
		fprintf(stderr, "SDL_SetWindowRelativeMouseMode() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
}

void ROSE_MouseReveal(void)
{
	mouse_xpos = saved_mouse_px;
	mouse_ypox = saved_mouse_py;
	SDL_WarpMouseInWindow(window, mouse_xpos, mouse_ypox);

	if (!SDL_SetWindowRelativeMouseMode(window, false))
	{
		fprintf(stderr, "SDL_SetWindowRelativeMouseMode() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
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

