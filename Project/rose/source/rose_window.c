
/*
 *   Source [rose_window.c]
 */

#include "rose.h"

void ROSE_WindowInit(const char* title, uInt32 width, uInt32 height)
{
	bool vkdebug = false;
	#if defined(ROSE_DEBUG)
	vkdebug = true;
	#endif

	const uInt32 screen_min = 256;
	rose_screen.width = (width < screen_min) ? screen_min : width;
	rose_screen.height = (height < screen_min) ? screen_min : height;

	SDL_Init(SDL_INIT_VIDEO);
	rose__window = SDL_CreateWindow(title, (int)rose_screen.width, (int)rose_screen.height, SDL_WINDOW_RESIZABLE);
	rose__device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, vkdebug, NULL);

	SDL_ClaimWindowForGPUDevice(rose__device, rose__window);
	SDL_SetGPUSwapchainParameters(rose__device, rose__window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
	SDL_SetWindowMinimumSize(rose__window, (int)screen_min, (int)screen_min);

	rose__sampler = ROSE__SDLCreateSampler();
	rose__sprite_pipeline = ROSE__SDLCreateGraphicsPipelineSprite();
	rose__triangles_pipeline = ROSE__SDLCreateGraphicsPipelineTriangles();
	rose__points_pipeline = ROSE__SDLCreateGraphicsPipelinePoints();
	rose__lines_pipeline = ROSE__SDLCreateGraphicsPipelineLines();

	ROSE_Vertex2Dtex quad_vertices[] = {
		{.xpos = 0.0f, .ypos =  0.0f, .xtex = 0.0f, .ytex = 1.0f },
		{.xpos = 1.0f, .ypos =  0.0f, .xtex = 1.0f, .ytex = 1.0f },
		{.xpos = 1.0f, .ypos = -1.0f, .xtex = 1.0f, .ytex = 0.0f },
		{.xpos = 0.0f, .ypos =  0.0f, .xtex = 0.0f, .ytex = 1.0f },
		{.xpos = 1.0f, .ypos = -1.0f, .xtex = 1.0f, .ytex = 0.0f },
		{.xpos = 0.0f, .ypos = -1.0f, .xtex = 0.0f, .ytex = 0.0f },
	};

	rose__sprite_quad_buffer = ROSE__SDLCreateVertexBuffer(
		quad_vertices,
		sizeof(quad_vertices) / sizeof(quad_vertices[0]),
		sizeof(ROSE_Vertex2Dtex)
	);

	ROSE_Image ascii_image = ROSE_ImageLoad("textures/ascii.png");
	ROSE_Image dummy_image = ROSE_ImageCreate(1, 1);
	rose__ascii_texture = ROSE__SDLCreateTexture(&ascii_image);
	rose__dummy_texture = ROSE__SDLCreateTexture(&dummy_image);
	ROSE_ImageDestroy(&ascii_image);
	ROSE_ImageDestroy(&dummy_image);
}

void ROSE_WindowShutdown(void)
{
	if (rose__device) {
		ROSE__SDLSubmitCopyPass();
		SDL_WaitForGPUIdle(rose__device);
		if (rose__sprite_quad_buffer) SDL_ReleaseGPUBuffer(rose__device, rose__sprite_quad_buffer);
		if (rose__sprite_pipeline)    SDL_ReleaseGPUGraphicsPipeline(rose__device, rose__sprite_pipeline);
		if (rose__triangles_pipeline) SDL_ReleaseGPUGraphicsPipeline(rose__device, rose__triangles_pipeline);
		if (rose__points_pipeline)    SDL_ReleaseGPUGraphicsPipeline(rose__device, rose__points_pipeline);
		if (rose__lines_pipeline)     SDL_ReleaseGPUGraphicsPipeline(rose__device, rose__lines_pipeline);
		if (rose__dummy_texture)      SDL_ReleaseGPUTexture(rose__device, rose__dummy_texture);
		if (rose__ascii_texture)      SDL_ReleaseGPUTexture(rose__device, rose__ascii_texture);
		if (rose__sampler)            SDL_ReleaseGPUSampler(rose__device, rose__sampler);
		if (rose__window)             SDL_ReleaseWindowFromGPUDevice(rose__device, rose__window);
		SDL_DestroyGPUDevice(rose__device);
	}

	if (rose__window)
		SDL_DestroyWindow(rose__window);

	SDL_Quit();
	rose_screen = (ROSE_ScreenInfo){ 0 };
	rose_mouse = (ROSE_MouseInfo){ 0 };

	rose__window        = NULL;
	rose__device        = NULL;
	rose__sampler       = NULL;
	rose__ascii_texture = NULL;
	rose__dummy_texture = NULL;

	rose__sprite_quad_buffer = NULL;
	rose__sprite_pipeline    = NULL;
	rose__triangles_pipeline = NULL;
	rose__points_pipeline    = NULL;
	rose__lines_pipeline     = NULL;

	rose__render_pass         = NULL;
	rose__swapchain_texture   = NULL;
	rose__command_buffer      = NULL;
	rose__copy_command_buffer = NULL;
	rose__copy_pass           = NULL;
	rose__bound_pipeline      = NULL;
	rose__bound_vertex_buffer = NULL;
	rose__bound_texture       = NULL;
	rose__present_mode        = SDL_GPU_PRESENTMODE_VSYNC;
	rose__present_mode_dirty  = false;

	rose__saved_mouse_px = 0.0f;
	rose__saved_mouse_py = 0.0f;
	rose__curr_mouse_state = 0u;
	rose__prev_mouse_state = 0u;
	memset(rose__curr_keyboard_state, false, sizeof(rose__curr_keyboard_state));
	memset(rose__prev_keyboard_state, false, sizeof(rose__prev_keyboard_state));
}

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

