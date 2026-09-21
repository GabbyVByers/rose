
/*
 *   Source File [rose_window.c]
 */

#include "rose.h"

#define ROSE_MIN_WIDTH 32
#define ROSE_MIN_HEIGHT 32

typedef struct ROSE_Window {
	i32 screen_width;
	i32 screen_height;
	SDL_Window* window;
	SDL_GPUDevice* device;
	SDL_GPUSampler* sampler;
	SDL_GPUTexture* depth_texture;
	SDL_GPUGraphicsPipeline* pipeline;
	SDL_GPUTexture* ascii_texture;
	bool frame;
	bool minimized;
	SDL_GPURenderPass* render_pass;
	SDL_GPUTexture* swapchain_texture;
	SDL_GPUCommandBuffer* command_buffer;
	float mouse_px;
	float mouse_py;
	float mouse_vx;
	float mouse_vy;
	float saved_mouse_px;
	float saved_mouse_py;
	int curr_mouse_state;
	int prev_mouse_state;
	float mouse_scroll;
	bool curr_keyboard_state[SDL_SCANCODE_COUNT];
	bool prev_keyboard_state[SDL_SCANCODE_COUNT];
} ROSE_Window;

ROSE_Window* ROSE_WindowCreate(const char* title, i32 width, i32 height, bool vkdebug) {

	ROSE_Window* window = malloc(sizeof(ROSE_Window));
	if (!window) {
		fprintf(stderr, "ROSE_WindowCreate() Failed\n");
		fprintf(stderr, "NULL Allocation\n");
		exit(EXIT_FAILURE);
	}

	window->screen_width = 0;
	window->screen_height = 0;

	window->window = NULL;
	window->device = NULL;
	window->sampler = NULL;
	window->depth_texture = NULL;
	window->pipeline = NULL;
	window->ascii_texture = NULL;

	window->frame = false;
	window->minimized = false;
	window->render_pass = NULL;
	window->swapchain_texture = NULL;
	window->command_buffer = NULL;

	window->mouse_px = 0.0f;
	window->mouse_py = 0.0f;
	window->mouse_vx = 0.0f;
	window->mouse_vy = 0.0f;
	window->saved_mouse_px = 0.0f;
	window->saved_mouse_py = 0.0f;
	window->curr_mouse_state = 0;
	window->prev_mouse_state = 0;
	window->mouse_scroll = 0.0f;

	for (i32 i = 0; i < SDL_SCANCODE_COUNT; i++) {
		window->curr_keyboard_state[SDL_SCANCODE_COUNT] = false;
		window->prev_keyboard_state[SDL_SCANCODE_COUNT] = false;
	}

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "SDL_Init() Failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	if (width < ROSE_MIN_WIDTH) { width = ROSE_MIN_WIDTH; }
	if (height < ROSE_MIN_HEIGHT) { height = ROSE_MIN_HEIGHT; }
	window->screen_width = width;
	window->screen_height = height;

	window->window = SDL_CreateWindow(title, window->screen_width, window->screen_height, SDL_WINDOW_RESIZABLE);
	if (!window->window) {
		SDL_Log("SDL_CreateWindow() Failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	window->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, vkdebug, NULL);
	if (!window->device) {
		SDL_Log("SDL_CreateGPUDevice() Failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	if (!SDL_ClaimWindowForGPUDevice(window->device, window)) {
		SDL_Log("SDL_ClaimWindowForGPUDevice() Failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	if (!SDL_SetGPUSwapchainParameters(window->device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC)) {
		SDL_Log("SDL_SetGPUSwapchainParameters() Failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	if (!SDL_SetWindowMinimumSize(window->window, ROSE_MIN_WIDTH, ROSE_MIN_HEIGHT)) {
		SDL_Log("SDL_SetWindowMinimumSize() Failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	const char* vertex_path = "Shaders/vertex.spv";
	const char* fragment_path = "Shaders/fragment.spv";

	FILE* vertex_file = fopen(vertex_path, "rb");
	FILE* fragment_file = fopen(fragment_path, "rb");

	if (!vertex_file) {
		fprintf(stderr, "ROSE_WindowCreate() Failed\n");
		fprintf(stderr, "Couldn't Open: %s\n", vertex_path);
		exit(EXIT_FAILURE);
	}

	if (!fragment_file) {
		fprintf(stderr, "ROSE_WindowCreate() Failed\n");
		fprintf(stderr, "Couldn't Open: %s\n", fragment_path);
		exit(EXIT_FAILURE);
	}

	fseek(vertex_file, 0, SEEK_END);
	fseek(fragment_file, 0, SEEK_END);
	usize vertex_code_size = ftell(vertex_file);
	usize fragment_code_size = ftell(fragment_file);
	rewind(vertex_file);
	rewind(fragment_file);

	u8* vertex_code = malloc(vertex_code_size);
	u8* fragment_code = malloc(fragment_code_size);

	if (!vertex_code) {
		fprintf(stderr, "ROSE_WindowCreate() Failed\n");
		fprintf(stderr, "NULL Allocation\n");
		exit(EXIT_FAILURE);
	}

	if (!fragment_code) {
		fprintf(stderr, "ROSE_WindowCreate() Failed\n");
		fprintf(stderr, "NULL Allocation\n");
		exit(EXIT_FAILURE);
	}

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

	SDL_GPUShader* vertex_shader_program = SDL_CreateGPUShader(window->device, &vertex_shader_create_info);
	if (!vertex_shader_program) {
		fprintf(stderr, "SDL_CreateGPUShader() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_GPUShader* fragment_shader_program = SDL_CreateGPUShader(window->device, &fragment_shader_create_info);
	if (!fragment_shader_program) {
		fprintf(stderr, "SDL_CreateGPUShader() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

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
		.format = SDL_GetGPUSwapchainTextureFormat(window->device, window),
		.blend_state = {
			.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
			.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.color_blend_op = SDL_GPU_BLENDOP_ADD,
			.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
			.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			.alpha_blend_op = SDL_GPU_BLENDOP_ADD,
			.enable_blend = true,
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
		.width = window->screen_width,
		.height = window->screen_height,
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
			.enable_depth_test = true,
			.enable_depth_write = true,
		},
		.target_info = {
			.color_target_descriptions = &color_target_description,
			.num_color_targets = 1,
			.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
			.has_depth_stencil_target = true,
		},
	};

	window->sampler = SDL_CreateGPUSampler(window->device, &sampler_create_info);
	if (!window->sampler) {
		fprintf(stderr, "SDL_CreateGPUSampler() Failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	window->depth_texture = SDL_CreateGPUTexture(window->device, &depth_texture_create_info);
	if (!window->depth_texture) {
		fprintf(stderr, "SDL_CreateGPUTexture() Failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	window->pipeline = SDL_CreateGPUGraphicsPipeline(window->device, &graphics_pipeline_create_info);
	if (!window->pipeline) {
		fprintf(stderr, "SDL_CreateGPUGraphicsPipeline() Failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	fclose(vertex_file);
	fclose(fragment_file);
	free(vertex_code);
	free(fragment_code);

	SDL_ReleaseGPUShader(window->device, vertex_shader_program);
	SDL_ReleaseGPUShader(window->device, fragment_shader_program);

	return window;
}

