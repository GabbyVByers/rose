
/*
 *   Source File [rose_window.c]
 */

#include "rose.h"

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
	const i32 min = 32;
	screen_width = (width < min) ? min : width;
	screen_height = (height < min) ? min : height;

	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow(title, screen_width, screen_height, SDL_WINDOW_RESIZABLE);
	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, vkdebug, NULL);

	SDL_ClaimWindowForGPUDevice(device, window);
	SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);
	SDL_SetWindowMinimumSize(window, min, min);

	FILE* vertex_file = fopen("Shaders/vertex.spv", "rb");
	FILE* fragment_file = fopen("Shaders/fragment.spv", "rb");
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

}

void ROSE_WindowToggleVSync(bool vsunc) {

}

void ROSE_WindowGetDimensions(i32* width, i32* height) {
	*width = screen_width;
	*height = screen_height;
}

bool ROSE_WindowPollEvents(void) {
	return TRUE;
}

void ROSE_WindowClearScreen(ROSE_Color color) {

}

void ROSE_WindowDrawSprite(ROSE_Sprite* sprite, i32 x, i32 y, double scale, ROSE_Color color) {

}

void ROSE_WindowDrawText(ROSE_Text* text, i32 x, i32 y, double scale, ROSE_Color color) {

}

void ROSE_WindowSwapBuffers(void) {

}

