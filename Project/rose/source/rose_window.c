
/*
 *   Source File [rose_window.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

void ROSE_WindowToggleVSync(bool vsync)
{
	if (vsync) {
		SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
		return;
	}

	const bool immediate = SDL_WindowSupportsGPUPresentMode(device, window, SDL_GPU_PRESENTMODE_IMMEDIATE);
	if (immediate) {
		SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE);
		return;
	}

	const bool mailbox = SDL_WindowSupportsGPUPresentMode(device, window, SDL_GPU_PRESENTMODE_MAILBOX);
	if (mailbox) {
		SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX);
		return;
	}

	SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
}

bool ROSE_WindowIsOpen(void)
{
	mouse_xvel = 0.0f;
	mouse_yvel = 0.0f;
	mouse_scroll = 0.0f;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			return false;

		if (event.type == SDL_EVENT_MOUSE_WHEEL)
			mouse_scroll += event.wheel.y;

		if (event.type == SDL_EVENT_MOUSE_MOTION) {
			mouse_xvel += event.motion.xrel;
			mouse_yvel += event.motion.yrel;
		}
	}

	memcpy(prev_keyboard_state, curr_keyboard_state, sizeof(bool) * SDL_SCANCODE_COUNT);
	memcpy(curr_keyboard_state, SDL_GetKeyboardState(NULL), sizeof(bool) * SDL_SCANCODE_COUNT);

	prev_mouse_state = curr_mouse_state;
	curr_mouse_state = SDL_GetMouseState(&mouse_xpos, &mouse_ypos);
	return true;
}

void ROSE_WindowClear(ROSE_Color color)
{
	uint32_t width = 0;
	uint32_t height = 0;
	command_buffer = SDL_AcquireGPUCommandBuffer(device);
	SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, window, &swapchain_texture, &width, &height);

	minimized = (width == 0) || (height == 0);
	if (minimized) {
		SDL_SubmitGPUCommandBuffer(command_buffer);
		return;
	}

	const bool resize_depth_texture =
		(width != screen_width) ||
		(height != screen_height);
	
	screen_width = width;
	screen_height = height;

	if (resize_depth_texture) {
		SDL_ReleaseGPUTexture(device, depth_texture);

		SDL_GPUTextureCreateInfo depth_texture_create_info = {
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
			.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
			.width = screen_width,
			.height = screen_height,
			.layer_count_or_depth = 1,
			.num_levels = 1,
			.sample_count = SDL_GPU_SAMPLECOUNT_1
		};

		depth_texture = SDL_CreateGPUTexture(device, &depth_texture_create_info);
	}

	SDL_GPUColorTargetInfo color_target_info = {
		.texture = swapchain_texture,
		.clear_color = {
			.r = color.r,
			.g = color.g,
			.b = color.b,
			.a = color.a,
		},
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
	};

	SDL_GPUDepthStencilTargetInfo depth_stencil_target_info = {
		.texture = depth_texture,
		.clear_depth = 1,
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_DONT_CARE,
		.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
		.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
		.cycle = true,
	};

	render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, &depth_stencil_target_info);
}

void ROSE_WindowRender(void)
{
	if (minimized)
		return;

	SDL_EndGPURenderPass(render_pass);
	SDL_SubmitGPUCommandBuffer(command_buffer);
}

