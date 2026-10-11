
/*
 *   Source File [rose_init.c]
 */

#include "rose.h"
#include "rose_state.h"

void ROSE_Init(const char* title, u32 width, u32 height, bool vkdebug)
{
	assert(width != 0);
	assert(height != 0);

	const u32 min = 256;
	screen_width = (width < min) ? min : width;
	screen_height = (height < min) ? min : height;

	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		fprintf(stderr, "SDL_Init() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	window = SDL_CreateWindow(title, (int)screen_width, (int)screen_height, SDL_WINDOW_RESIZABLE);

	if (!window)
	{
		fprintf(stderr, "SDL_CreateWindow() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, vkdebug, NULL);

	if (!device)
	{
		fprintf(stderr, "SDL_CreateGPUDevice() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	if (!SDL_ClaimWindowForGPUDevice(device, window))
	{
		fprintf(stderr, "SDL_ClaimWindowForGPUDevice() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	if (!SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC))
	{
		fprintf(stderr, "SDL_SetGPUSwapchainParameters() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	if (!SDL_SetWindowMinimumSize(window, (int)min, (int)min))
	{
		fprintf(stderr, "SDL_SetWindowMinimumSize() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	sampler = ROSE_SDLCreateSampler();
	depth_texture = ROSE_SDLCreateDepthTexture();
	sprite_pipeline = ROSE_SDLCreateGraphicsPipelineSprite();
	mesh_pipeline = ROSE_SDLCreateGraphicsPipelineMesh();

	ROSE_Image* ascii_image = ROSE_ImageLoad("textures/ascii.png");
	ascii_texture = ROSE_SDLCreateTexture(ascii_image);
	ROSE_ImageDestroy(ascii_image);
}

void ROSE_Quit(void)
{
	// destroy resources...
}

