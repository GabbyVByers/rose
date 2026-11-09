
/*
 *   Source [rose_window.c]
 */

#include "rose.h"

void ROSE_WindowVSync(bool vsync)
{
	rose__present_mode_dirty = true;
	rose__present_mode = SDL_GPU_PRESENTMODE_VSYNC;
	
	if (vsync) {
		return;
	}

	if (SDL_WindowSupportsGPUPresentMode(rose__device, rose__window, SDL_GPU_PRESENTMODE_IMMEDIATE)) {
		rose__present_mode = SDL_GPU_PRESENTMODE_IMMEDIATE;
		return;
	}

	if (SDL_WindowSupportsGPUPresentMode(rose__device, rose__window, SDL_GPU_PRESENTMODE_MAILBOX)) {
		rose__present_mode = SDL_GPU_PRESENTMODE_MAILBOX;
		return;
	}
}

bool ROSE_WindowIsOpen(void)
{
	const Int32 minimized_timeout_ms = 50;
	if (rose_screen.minimized) {
		SDL_WaitEventTimeout(NULL, minimized_timeout_ms);
	}

	rose_mouse.xvel = 0.0f;
	rose_mouse.yvel = 0.0f;
	rose_mouse.scroll = 0.0f;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case (SDL_EVENT_WINDOW_CLOSE_REQUESTED): {
				return false;
			}
			break;
			case (SDL_EVENT_QUIT): {
				return false;
			}
			break;
			case (SDL_EVENT_MOUSE_WHEEL): {
				rose_mouse.scroll += event.wheel.y;
			}
			break;
			case (SDL_EVENT_MOUSE_MOTION): {
				rose_mouse.xvel += event.motion.xrel;
				rose_mouse.yvel += event.motion.yrel;
			}
			break;
			default: break;
		}
	}

	memcpy(rose__prev_keyboard_state, rose__curr_keyboard_state, sizeof(bool) * SDL_SCANCODE_COUNT);
	memcpy(rose__curr_keyboard_state, SDL_GetKeyboardState(NULL), sizeof(bool) * SDL_SCANCODE_COUNT);

	rose__prev_mouse_state = rose__curr_mouse_state;
	rose__curr_mouse_state = SDL_GetMouseState(&rose_mouse.xpos, &rose_mouse.ypos);
	return true;
}

void ROSE_WindowFrameClear(uInt32 color)
{
	if (rose__render_pass) {
		ROSE_PANIC("ROSE_WindowFrameClear() called twice without ROSE_WindowFrameRender()!");
	}

	if (rose__present_mode_dirty) {
		SDL_SetGPUSwapchainParameters(rose__device, rose__window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, rose__present_mode);
	}

	rose__present_mode_dirty = false;
	rose__command_buffer = SDL_AcquireGPUCommandBuffer(rose__device);

	uInt32 width = 0;
	uInt32 height = 0;
	SDL_WaitAndAcquireGPUSwapchainTexture(rose__command_buffer, rose__window, &rose__swapchain_texture, &width, &height);
	
	rose_screen.minimized = (rose__swapchain_texture == NULL);
	if (rose_screen.minimized) {
		SDL_SubmitGPUCommandBuffer(rose__command_buffer);
		return;
	}

	rose_screen.width = width;
	rose_screen.height = height;

	float r = ((float)((uInt8)(color >> 24))) / 255.0f;
	float g = ((float)((uInt8)(color >> 16))) / 255.0f;
	float b = ((float)((uInt8)(color >> 8)))  / 255.0f;
	float a = ((float)((uInt8)(color)))       / 255.0f;

	SDL_GPUColorTargetInfo color_target_info = {
		.texture = rose__swapchain_texture,
		.clear_color = {
			.r = r,
			.g = g,
			.b = b,
			.a = a,
		},
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
	};

	rose__render_pass = SDL_BeginGPURenderPass(rose__command_buffer, &color_target_info, 1, NULL);
	rose__bound_pipeline = NULL;
	rose__bound_vertex_buffer = NULL;
	rose__bound_texture = NULL;
}

void ROSE_WindowFrameRender(void)
{
	ROSE__SDLSubmitCopyPass();
	if (rose_screen.minimized) {
		return;
	}

	if (!rose__render_pass) {
		ROSE_PANIC("ROSE_WindowFrameRender() called without ROSE_WindowFrameClear()!");
	}

	SDL_EndGPURenderPass(rose__render_pass);
	SDL_SubmitGPUCommandBuffer(rose__command_buffer);
	rose__render_pass = NULL;
}

