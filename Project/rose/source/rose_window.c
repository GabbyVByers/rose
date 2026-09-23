
/*
 *   Source File [rose_window.c]
 */

#include "rose.h"

typedef struct ROSE_Sprite {
	i32 w, h;
	SDL_GPUBuffer* buffer;
	SDL_GPUTexture* texture;
} ROSE_Sprite;

typedef struct ROSE_Text {
	i32 w, h;
	usize num_vertices;
	SDL_GPUBuffer* buffer;
} ROSE_Text;

static i32 screen_width = 0;
static i32 screen_height = 0;
static SDL_Window* window = NULL;
static SDL_GPUDevice* device = NULL;
static SDL_GPUSampler* sampler = NULL;
static SDL_GPUTexture* depth_texture = NULL;
static SDL_GPUGraphicsPipeline* pipeline = NULL;
static SDL_GPUTexture* ascii_texture = NULL;

static bool minimized = FALSE;
static SDL_GPURenderPass* render_pass = NULL;
static SDL_GPUTexture* swapchain_texture = NULL;
static SDL_GPUCommandBuffer* command_buffer = NULL;

static float mouse_px = 0.0f;
static float mouse_py = 0.0f;
static float mouse_vx = 0.0f;
static float mouse_vy = 0.0f;
static float saved_mouse_px = 0.0f;
static float saved_mouse_py = 0.0f;
static i32 curr_mouse_state = 0;
static i32 prev_mouse_state = 0;
static float mouse_scroll = 0.0f;

static bool curr_keyboard_state[SDL_SCANCODE_COUNT] = { FALSE };
static bool prev_keyboard_state[SDL_SCANCODE_COUNT] = { FALSE };

void ROSE_Init(const char* title, i32 width, i32 height, bool vkdebug) {
	const i32 min = 256;
	screen_width = (width < min) ? min : width;
	screen_height = (height < min) ? min : height;

	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(title, screen_width, screen_height, SDL_WINDOW_RESIZABLE);
	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, vkdebug, NULL);

	SDL_ClaimWindowForGPUDevice(device, window);
	SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
	SDL_SetWindowMinimumSize(window, min, min);

	FILE* vertex_file = fopen("shaders/vertex.spv", "rb");
	FILE* fragment_file = fopen("shaders/fragment.spv", "rb");
	assert(vertex_file);
	assert(fragment_file);

	fseek(vertex_file, 0, SEEK_END);
	fseek(fragment_file, 0, SEEK_END);
	usize vertex_code_size = ftell(vertex_file);
	usize fragment_code_size = ftell(fragment_file);
	rewind(vertex_file);
	rewind(fragment_file);

	u8* vertex_code = malloc(vertex_code_size);
	u8* fragment_code = malloc(fragment_code_size);
	assert(vertex_code);
	assert(fragment_code);
	
	fread(vertex_code, 1, vertex_code_size, vertex_file);
	fread(fragment_code, 1, fragment_code_size, fragment_file);

	SDL_GPUShaderCreateInfo vertex_shader_create_info = {
		.code_size = vertex_code_size,
		.code = vertex_code,
		.entrypoint = "main",
		.format = SDL_GPU_SHADERFORMAT_SPIRV,
		.stage = SDL_GPU_SHADERSTAGE_VERTEX,
		.num_uniform_buffers = 1,
	};

	SDL_GPUShaderCreateInfo fragment_shader_create_info = {
		.code_size = fragment_code_size,
		.code = fragment_code,
		.entrypoint = "main",
		.format = SDL_GPU_SHADERFORMAT_SPIRV,
		.stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
		.num_samplers = 1,
	};

	SDL_GPUShader* vertex_shader_program = SDL_CreateGPUShader(device, &vertex_shader_create_info);
	SDL_GPUShader* fragment_shader_program = SDL_CreateGPUShader(device, &fragment_shader_create_info);

	SDL_GPUVertexAttribute position_attribute = {
		.location = 0,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
		.offset = offsetof(ROSE_Vertex, pos),
	};

	SDL_GPUVertexAttribute texcoords_attribute = {
		.location = 1,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
		.offset = offsetof(ROSE_Vertex, uv),
	};

	SDL_GPUVertexAttribute vertex_attributes[2] = {
		position_attribute,
		texcoords_attribute,
	};

	SDL_GPUVertexBufferDescription vertex_buffer_description = {
		.pitch = sizeof(ROSE_Vertex),
		.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
	};

	SDL_GPUColorTargetDescription color_target_description = {
		.format = SDL_GetGPUSwapchainTextureFormat(device, window),
		.blend_state = {
			.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
			.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.color_blend_op = SDL_GPU_BLENDOP_ADD,
			.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
			.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.alpha_blend_op = SDL_GPU_BLENDOP_ADD,
			.enable_blend = TRUE,
		},
	};

	SDL_GPUSamplerCreateInfo sampler_create_info = {
		.min_filter = SDL_GPU_FILTER_NEAREST,
		.mag_filter = SDL_GPU_FILTER_NEAREST,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
	};

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

	SDL_GPUGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
		.vertex_shader = vertex_shader_program,
		.fragment_shader = fragment_shader_program,
		.vertex_input_state = {
			.vertex_buffer_descriptions = &vertex_buffer_description,
			.num_vertex_buffers = 1,
			.vertex_attributes = vertex_attributes,
			.num_vertex_attributes = sizeof(vertex_attributes) / sizeof(vertex_attributes[0]),
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.depth_stencil_state = {
			.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
			.enable_depth_test = TRUE,
			.enable_depth_write = TRUE,
		},
		.target_info = {
			.color_target_descriptions = &color_target_description,
			.num_color_targets = 1,
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
			.has_depth_stencil_target = TRUE,
		},
	};

	sampler = SDL_CreateGPUSampler(device, &sampler_create_info);
	depth_texture = SDL_CreateGPUTexture(device, &depth_texture_create_info);
	pipeline = SDL_CreateGPUGraphicsPipeline(device, &graphics_pipeline_create_info);

	fclose(vertex_file);
	fclose(fragment_file);
	free(vertex_code);
	free(fragment_code);
	
	SDL_ReleaseGPUShader(device, vertex_shader_program);
	SDL_ReleaseGPUShader(device, fragment_shader_program);
}

void ROSE_Quit(void) {
	SDL_ReleaseGPUTexture(device, ascii_texture);
	SDL_ReleaseGPUSampler(device, sampler);
	SDL_ReleaseGPUTexture(device, depth_texture);
	SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
	SDL_DestroyGPUDevice(device);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void ROSE_WindowToggleVSync(bool vsync) {
	if (vsync) {
		SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
	}
	
	else if (SDL_WindowSupportsGPUPresentMode(device, window, SDL_GPU_PRESENTMODE_IMMEDIATE)) {
		SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE);
	}
	
	else if (SDL_WindowSupportsGPUPresentMode(device, window, SDL_GPU_PRESENTMODE_MAILBOX)) {
		SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX);
	}
	
	else {
		SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
	}
}

bool ROSE_WindowIsOpen(void) {
	mouse_vx = 0.0f;
	mouse_vy = 0.0f;
	mouse_scroll = 0.0f;
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) { return FALSE; }
		if (event.type == SDL_EVENT_MOUSE_WHEEL) { mouse_scroll += event.wheel.y; }
		if (event.type == SDL_EVENT_MOUSE_MOTION) {
			mouse_vx += event.motion.xrel;
			mouse_vy += event.motion.yrel;
		}
	}

	memcpy(prev_keyboard_state, curr_keyboard_state, sizeof(bool) * SDL_SCANCODE_COUNT);
	memcpy(curr_keyboard_state, SDL_GetKeyboardState(NULL), sizeof(bool) * SDL_SCANCODE_COUNT);
	prev_mouse_state = curr_mouse_state;
	curr_mouse_state = SDL_GetMouseState(&mouse_px, &mouse_py);
	return TRUE;
}

void ROSE_WindowClearScreen(ROSE_Color color) {
	unsigned int new_width, new_height;
	command_buffer = SDL_AcquireGPUCommandBuffer(device);
	SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, window, &swapchain_texture, &new_width, &new_height);
	
	usize new_screen_width = (usize)new_width;
	usize new_screen_height = (usize)new_height;
	minimized = (new_screen_width == 0) || (new_screen_height == 0);
	if (minimized) {
		SDL_SubmitGPUCommandBuffer(command_buffer);
		return;
	}

	const bool resize_depth_texture = (new_screen_width != screen_width) || (new_screen_height != screen_height);
	screen_width = new_screen_width;
	screen_height = new_screen_height;

	if (resize_depth_texture) {
		SDL_ReleaseGPUTexture(device, depth_texture);
		depth_texture = ROSE_INTERNAL_CreateDepthTexture();
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
		.cycle = TRUE
	};

	render_pass = SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, &depth_stencil_target_info);
	SDL_BindGPUGraphicsPipeline(render_pass, pipeline);
}

void ROSE_WindowDrawSprite(ROSE_Sprite* sprite, i32 px, i32 py, double scale, ROSE_Color color) {
	SDL_GPUBufferBinding buffer_binding = {
		.buffer = sprite->buffer,
	};

	SDL_GPUTextureSamplerBinding texture_binding = {
		.texture = sprite->texture,
		.sampler = sampler,
	};

	double scaled_width = (double)sprite->w * scale;
	double scaled_height = (double)sprite->h * scale;

	double ww = (scaled_width / (double)screen_width) * 2.0;
	double hh = (scaled_height / (double)screen_height) * 2.0;
	double xx = -1.0 + (((double)px / (double)screen_width) * 2.0);
	double yy = 1.0 - (((double)py / (double)screen_height) * 2.0);

	float w = (float)ww;
	float h = (float)hh;
	float x = (float)xx;
	float y = (float)yy;

	typedef struct Uniform {
		float TransformMatrix[16];
		float TintColor[4];
	} Uniform;

	Uniform uniform = {
		.TransformMatrix = {
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, 1, 0,
			x, y, 0, 1,
		},
		.TintColor = {
			color.r,
			color.g,
			color.b,
			color.a,
		}
	};

	SDL_PushGPUVertexUniformData(command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);
	SDL_BindGPUFragmentSamplers(render_pass, 0, &texture_binding, 1);
	SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);
}

void ROSE_WindowDrawText(ROSE_Text* text, i32 px, i32 py, double scale, ROSE_Color color) {
	SDL_GPUBufferBinding buffer_binding = {
		.buffer = text->buffer,
	};

	SDL_GPUTextureSamplerBinding texture_binding = {
		.texture = ascii_texture,
		.sampler = sampler,
	};

	double scaled_width = (double)text->w * scale;
	double scaled_height = (double)text->h * scale;

	double ww = (scaled_width / (double)screen_width) * 2.0;
	double hh = (scaled_height / (double)screen_height) * 2.0;
	double xx = -1.0 + (((double)px / (double)screen_width) * 2.0);
	double yy = 1.0 - (((double)py / (double)screen_height) * 2.0);

	float w = (float)ww;
	float h = (float)hh;
	float x = (float)xx;
	float y = (float)yy;

	typedef struct Uniform {
		float TransformMatrix[16];
		float TintColor[4];
	} Uniform;

	Uniform uniform = {
		.TransformMatrix = {
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, 1, 0,
			x, y, 0, 1,
		},
		.TintColor = {
			color.r,
			color.g,
			color.b,
			color.a,
		}
	};

	SDL_PushGPUVertexUniformData(command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);
	SDL_BindGPUFragmentSamplers(render_pass, 0, &texture_binding, 1);
	SDL_DrawGPUPrimitives(render_pass, text->num_vertices, 1, 0, 0);
}

void ROSE_WindowRender(void) {
	if (minimized) { return; }
	SDL_EndGPURenderPass(render_pass);
	SDL_SubmitGPUCommandBuffer(command_buffer);
}

SDL_GPUTexture* ROSE_INTERNAL_CreateDepthTexture(void) {
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

	SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &depth_texture_create_info);
	return texture;
}

