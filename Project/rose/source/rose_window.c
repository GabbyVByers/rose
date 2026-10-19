
/*
 *   Source File [rose_window.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

void ROSE_WindowToggleVSync(bool vsync)
{
	if (vsync) {
		SDL_SetGPUSwapchainParameters(rose__device, rose__window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
		return;
	}

	const bool immediate = SDL_WindowSupportsGPUPresentMode(rose__device, rose__window, SDL_GPU_PRESENTMODE_IMMEDIATE);
	if (immediate) {
		SDL_SetGPUSwapchainParameters(rose__device, rose__window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE);
		return;
	}

	const bool mailbox = SDL_WindowSupportsGPUPresentMode(rose__device, rose__window, SDL_GPU_PRESENTMODE_MAILBOX);
	if (mailbox) {
		SDL_SetGPUSwapchainParameters(rose__device, rose__window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX);
		return;
	}

	SDL_SetGPUSwapchainParameters(rose__device, rose__window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
}

bool ROSE_WindowPollEvents(void)
{
	rose_mouse.xvel = 0.0f;
	rose_mouse.yvel = 0.0f;
	rose_mouse.scroll = 0.0f;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			return false;

		if (event.type == SDL_EVENT_MOUSE_WHEEL)
			rose_mouse.scroll += event.wheel.y;

		if (event.type == SDL_EVENT_MOUSE_MOTION) {
			rose_mouse.xvel += event.motion.xrel;
			rose_mouse.yvel += event.motion.yrel;
		}
	}

	memcpy(rose__prev_keyboard_state, rose__curr_keyboard_state, sizeof(bool) * SDL_SCANCODE_COUNT);
	memcpy(rose__curr_keyboard_state, SDL_GetKeyboardState(NULL), sizeof(bool) * SDL_SCANCODE_COUNT);

	rose__prev_mouse_state = rose__curr_mouse_state;
	rose__curr_mouse_state = SDL_GetMouseState(&rose_mouse.xpos, &rose_mouse.ypos);
	return true;
}

void ROSE_WindowBeginFrame(ROSE_Color clear_color)
{
	uint32_t width = 0;
	uint32_t height = 0;
	rose__command_buffer = SDL_AcquireGPUCommandBuffer(rose__device);

	bool acquired = SDL_WaitAndAcquireGPUSwapchainTexture(rose__command_buffer, rose__window, &rose__swapchain_texture, &width, &height);
	rose_screen.minimized = (!acquired) || (rose__swapchain_texture == NULL);

	if (rose_screen.minimized) {
		SDL_SubmitGPUCommandBuffer(rose__command_buffer);
		return;
	}

	rose_screen.width = width;
	rose_screen.height = height;

	SDL_GPUColorTargetInfo color_target_info = {
		.texture = rose__swapchain_texture,
		.clear_color = {
			.r = clear_color.r,
			.g = clear_color.g,
			.b = clear_color.b,
			.a = clear_color.a,
		},
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
	};

	rose__render_pass = SDL_BeginGPURenderPass(rose__command_buffer, &color_target_info, 1, NULL);
}

void ROSE_WindowEndFrame(void)
{
	if (rose_screen.minimized)
		return;

	SDL_EndGPURenderPass(rose__render_pass);
	SDL_SubmitGPUCommandBuffer(rose__command_buffer);
}

