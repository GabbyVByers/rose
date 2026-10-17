
/*
 *   Source File [rose_window.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

void ROSE_WindowToggleVSync(bool vsync)
{
	if (vsync)
	{
		if (!SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC))
		{
			fprintf(stderr, "SDL_SetGPUSwapchainParameters() Failed: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}

		return;
	}

	if (SDL_WindowSupportsGPUPresentMode(device, window, SDL_GPU_PRESENTMODE_IMMEDIATE))
	{
		if (!SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE))
		{
			fprintf(stderr, "SDL_SetGPUSwapchainParameters() Failed: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}

		return;
	}

	if (SDL_WindowSupportsGPUPresentMode(device, window, SDL_GPU_PRESENTMODE_MAILBOX))
	{
		if (!SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX))
		{
			fprintf(stderr, "SDL_SetGPUSwapchainParameters() Failed: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}

		return;
	}

	if (!SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC))
	{
		fprintf(stderr, "SDL_SetGPUSwapchainParameters() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
}

bool ROSE_WindowIsOpen(void)
{
	mouse_xvel = 0.0f;
	mouse_yvel = 0.0f;
	ROSE_MOUSE_SCROLL = 0.0f;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
		{
			return false;
		}

		if (event.type == SDL_EVENT_MOUSE_WHEEL)
		{
			ROSE_MOUSE_SCROLL += event.wheel.y;
		}

		if (event.type == SDL_EVENT_MOUSE_MOTION)
		{
			mouse_xvel += event.motion.xrel;
			mouse_yvel += event.motion.yrel;
		}
	}

	memcpy(prev_keyboard_state, curr_keyboard_state, sizeof(bool) * SDL_SCANCODE_COUNT);
	memcpy(curr_keyboard_state, SDL_GetKeyboardState(NULL), sizeof(bool) * SDL_SCANCODE_COUNT);

	prev_mouse_state = curr_mouse_state;
	curr_mouse_state = SDL_GetMouseState(&mouse_xpos, &mouse_ypox);
	return true;
}

void ROSE_WindowClear(float color[4])
{
	command_buffer = SDL_AcquireGPUCommandBuffer(device);

	if (!command_buffer)
	{
		fprintf(stderr, "SDL_AcquireGPUCommandBuffer() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	u32 width = 0;
	u32 height = 0;

	if (!SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, window, &swapchain_texture, &width, &height))
	{
		fprintf(stderr, "SDL_AcquireGPUCommandBuffer() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	minimized = (width == 0) || (height == 0);

	if (minimized)
	{
		if (!SDL_SubmitGPUCommandBuffer(command_buffer))
		{
			fprintf(stderr, "SDL_SubmitGPUCommandBuffer() Failed: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}

		return;
	}

	const bool resize_depth_texture = (width != screen_width) || (height != screen_height);
	screen_width = width;
	screen_height = height;

	if (resize_depth_texture)
	{
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

		if (!depth_texture)
		{
			fprintf(stderr, "SDL_CreateGPUTexture() Failed: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
	}

	SDL_GPUColorTargetInfo color_target_info = {
		.texture = swapchain_texture,
		.clear_color = {
			.r = color[0],
			.g = color[1],
			.b = color[2],
			.a = color[3],
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

	if (!render_pass)
	{
		fprintf(stderr, "SDL_BeginGPURenderPass() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
}

void ROSE_WindowRender(void)
{
	if (minimized)
	{
		return;
	}

	SDL_EndGPURenderPass(render_pass);

	if (!SDL_SubmitGPUCommandBuffer(command_buffer))
	{
		fprintf(stderr, "SDL_SubmitGPUCommandBuffer() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
}

