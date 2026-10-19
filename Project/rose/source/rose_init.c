
/*
 *   Source File [rose_init.c]
 */

#include "rose.h"
#include "rose_state.h"

void ROSE_Init(const char* title, uint32_t screen_width, uint32_t screen_height)
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

	bool vkdebug = false;
	#if defined(ROSE_DEBUG)
	vkdebug = true;
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
	rose__sprite_pipeline = ROSE__SDLCreateGraphicsPipelineSprite();
	rose__triangles_pipeline = ROSE__SDLCreateGraphicsPipelineTriangles();
	rose__points_pipeline = ROSE__SDLCreateGraphicsPipelinePoints();
	rose__lines_pipeline = ROSE__SDLCreateGraphicsPipelineLines();

	ROSE_VertexSprite quad_vertices[] = {
		{ { 0, 0 }, { 0, 1 } },
		{ { 1, 0 }, { 1, 1 } },
		{ { 1,-1 }, { 1, 0 } },
		{ { 0, 0 }, { 0, 1 } },
		{ { 1,-1 }, { 1, 0 } },
		{ { 0,-1 }, { 0, 0 } },
	};

	rose__sprite_quad_buffer = ROSE__SDLCreateVertexBuffer(quad_vertices, sizeof(quad_vertices) / sizeof(quad_vertices[0]), sizeof(ROSE_VertexSprite));

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
		if (rose__points_pipeline) SDL_ReleaseGPUGraphicsPipeline(rose__device, rose__points_pipeline);
		if (rose__sprite_pipeline) SDL_ReleaseGPUGraphicsPipeline(rose__device, rose__sprite_pipeline);
		if (rose__triangles_pipeline) SDL_ReleaseGPUGraphicsPipeline(rose__device, rose__triangles_pipeline);
		if (rose__lines_pipeline) SDL_ReleaseGPUGraphicsPipeline(rose__device, rose__lines_pipeline);
		if (rose__sprite_quad_buffer) SDL_ReleaseGPUBuffer(rose__device, rose__sprite_quad_buffer);
		if (rose__dummy_texture) SDL_ReleaseGPUTexture(rose__device, rose__dummy_texture);
		if (rose__ascii_texture) SDL_ReleaseGPUTexture(rose__device, rose__ascii_texture);
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
	rose__ascii_texture = NULL;
	rose__dummy_texture = NULL;
	rose__points_pipeline = NULL;
	rose__sprite_pipeline = NULL;
	rose__triangles_pipeline = NULL;
	rose__lines_pipeline = NULL;
	rose__sprite_quad_buffer = NULL;
	rose__render_pass = NULL;
	rose__swapchain_texture = NULL;
	rose__command_buffer = NULL;
}

