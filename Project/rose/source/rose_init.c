
/*
 *   Source File [rose_init.c]
 */

#include "rose.h"
#include "rose_state.h"

void ROSE_Init(const char* title, uint32_t width, uint32_t height, bool vkdebug)
{
	const uint32_t min = 256;
	screen_width = (width < min) ? min : width;
	screen_height = (height < min) ? min : height;
	
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(title, (int)screen_width, (int)screen_height, SDL_WINDOW_RESIZABLE);
	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, vkdebug, NULL);

	SDL_ClaimWindowForGPUDevice(device, window);
	SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
	SDL_SetWindowMinimumSize(window, (int)min, (int)min);

	sampler = ROSE_SDLCreateSampler();
	depth_texture = ROSE_SDLCreateDepthTexture();
	sprite_pipeline = ROSE_SDLCreateGraphicsPipelineSprite();

	ROSE_Image ascii_image = ROSE_ImageLoadFromFile("textures/ascii.png");
	ascii_texture = ROSE_SDLCreateTexture(&ascii_image);
	ROSE_ImageDestroy(&ascii_image);

	ROSE_Image dummy_image = ROSE_ImageCreate(1, 1);
	dummy_texture = ROSE_SDLCreateTexture(&dummy_image);
	ROSE_ImageDestroy(&dummy_image);
}

void ROSE_Quit(void)
{
	SDL_Quit();
}

