
/*
 *   Source File [rose_init.c]
 */

#include "rose.h"
#include "rose_state.h"

void ROSE_Init(const char* title, uint32_t screen_width, uint32_t screen_height, bool vkdebug)
{
	#ifdef ROSE_CRTDBG
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_DELAY_FREE_MEM_DF);
	#endif

	const uint32_t min = 256;
	rose_screen.width = (screen_width < min) ? min : screen_width;
	rose_screen.height = (screen_height < min) ? min : screen_height;

	SDL_Init(SDL_INIT_VIDEO);
	rose__window = SDL_CreateWindow(title, (int)rose_screen.width, (int)rose_screen.height, SDL_WINDOW_RESIZABLE);
	rose__device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, vkdebug, NULL);

	SDL_ClaimWindowForGPUDevice(rose__device, rose__window);
	SDL_SetGPUSwapchainParameters(rose__device, rose__window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
	SDL_SetWindowMinimumSize(rose__window, (int)min, (int)min);

	rose__sampler = ROSE__SDLCreateSampler();
	//rose__depth_texture = ROSE__SDLCreateDepthTexture();
	rose__sprite_pipeline = ROSE__SDLCreateGraphicsPipelineSprite();

	ROSE_Image ascii_image = ROSE_ImageLoadFromFile("textures/ascii.png");
	rose__ascii_texture = ROSE__SDLCreateTexture(&ascii_image);
	ROSE_ImageDestroy(&ascii_image);

	ROSE_Image dummy_image = ROSE_ImageCreate(1, 1);
	rose__dummy_texture = ROSE__SDLCreateTexture(&dummy_image);
	ROSE_ImageDestroy(&dummy_image);
}

void ROSE_Quit(void)
{
	if (rose__device) {
		SDL_WaitForGPUIdle(rose__device);
		if (rose__sprite_pipeline) SDL_ReleaseGPUGraphicsPipeline(rose__device, rose__sprite_pipeline);
		if (rose__dummy_texture) SDL_ReleaseGPUTexture(rose__device, rose__dummy_texture);
		if (rose__ascii_texture) SDL_ReleaseGPUTexture(rose__device, rose__ascii_texture);
		//if (rose__depth_texture) SDL_ReleaseGPUTexture(rose__device, rose__depth_texture);
		if (rose__sampler) SDL_ReleaseGPUSampler(rose__device, rose__sampler);
		if (rose__window) SDL_ReleaseWindowFromGPUDevice(rose__device, rose__window);
		SDL_DestroyGPUDevice(rose__device);
	}

	if (rose__window)
		SDL_DestroyWindow(rose__window);
	
	SDL_Quit();
	rose__window = NULL;
	rose__device = NULL;
	rose__sampler = NULL;
	//rose__depth_texture = NULL;
	rose__ascii_texture = NULL;
	rose__dummy_texture = NULL;
	rose__sprite_pipeline = NULL;
	rose__render_pass = NULL;
	rose__swapchain_texture = NULL;
	rose__command_buffer = NULL;
}

