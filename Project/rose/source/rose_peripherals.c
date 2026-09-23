
/*
 *   Source File [rose_peripherals.c]
 */

#include "rose.h"

extern SDL_Window* window;

float mouse_px = 0.0f;
float mouse_py = 0.0f;
float mouse_vx = 0.0f;
float mouse_vy = 0.0f;
float saved_mouse_px = 0.0f;
float saved_mouse_py = 0.0f;
i32 curr_mouse_state = 0;
i32 prev_mouse_state = 0;
float mouse_scroll = 0.0f;

bool curr_keyboard_state[SDL_SCANCODE_COUNT] = { FALSE };
bool prev_keyboard_state[SDL_SCANCODE_COUNT] = { FALSE };

/* Mouse */

bool ROSE_MouseHide(void) {
	saved_mouse_px = mouse_px;
	saved_mouse_py = mouse_py;
	SDL_SetWindowRelativeMouseMode(window, TRUE);
}

bool ROSE_MouseReveal(void) {
	mouse_px = saved_mouse_px;
	mouse_py = saved_mouse_py;
	SDL_WarpMouseInWindow(window, mouse_px, mouse_py);
	SDL_SetWindowRelativeMouseMode(window, FALSE);
}

bool ROSE_MousePressing(i32 button) {
	assert(ROSE_MOUSE_MIN <= button);
	assert(button <= ROSE_MOUSE_MAX);
	return (curr_mouse_state & SDL_BUTTON_MASK(button)) != 0;
}

bool ROSE_MousePressed(i32 button) {
	assert(ROSE_MOUSE_MIN <= button);
	assert(button <= ROSE_MOUSE_MAX);
	return ((prev_mouse_state & SDL_BUTTON_MASK(button)) == 0) && ((curr_mouse_state & SDL_BUTTON_MASK(button)) != 0);
}

bool ROSE_MouseReleased(i32 button) {
	assert(ROSE_MOUSE_MIN <= button);
	assert(button <= ROSE_MOUSE_MAX);
	return ((prev_mouse_state & SDL_BUTTON_MASK(button)) != 0) && ((curr_mouse_state & SDL_BUTTON_MASK(button)) == 0);
}

void ROSE_MousePosition(i32* px, i32* py) {
	assert(px);
	assert(py);
	*px = (i32)mouse_px;
	*py = (i32)mouse_py;
}

void ROSE_MouseVelocity(i32* vx, i32* vy) {
	assert(vx);
	assert(vy);
	*vx = (i32)mouse_vx;
	*vy = (i32)mouse_vy;
}

i32  ROSE_MouseScroll(void) {
	return (i32)mouse_scroll;
}

/* Keyboard */

bool ROSE_KeyboardPressing(i32 button) {
	assert(ROSE_KEYBOARD_MIN <= button);
	assert(button <= ROSE_KEYBOARD_MAX);
	return curr_keyboard_state[button];
}

bool ROSE_KeyboardPressed(i32 button) {
	assert(ROSE_KEYBOARD_MIN <= button);
	assert(button <= ROSE_KEYBOARD_MAX);
	return (!prev_keyboard_state[button]) && (curr_keyboard_state[button]);
}

bool ROSE_KeyboardReleased(i32 button) {
	assert(ROSE_KEYBOARD_MIN <= button);
	assert(button <= ROSE_KEYBOARD_MAX);
	return (prev_keyboard_state[button]) && (!curr_keyboard_state[button]);
}

