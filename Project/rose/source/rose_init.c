
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

	const char* vertex_path = "shaders/vertex.spv";
	const char* fragment_path = "shaders/fragment.spv";
	FILE* vertex_file = fopen(vertex_path, "rb");
	FILE* fragment_file = fopen(fragment_path, "rb");

	if (!vertex_file)
	{
		fprintf(stderr, "fopen() Failed: %s\n", vertex_path);
		exit(EXIT_FAILURE);
	}

	if (!fragment_file)
	{
		fprintf(stderr, "fopen() Failed: %s\n", fragment_path);
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
		.offset = offsetof(ROSE_Vertex2D, position),
	};

	SDL_GPUVertexAttribute texcoords_attribute = {
		.location = 1,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
		.offset = offsetof(ROSE_Vertex2D, texcoords),
	};

	SDL_GPUVertexAttribute vertex_attributes[2] = {
		position_attribute,
		texcoords_attribute,
	};

	SDL_GPUVertexBufferDescription vertex_buffer_description = {
		.pitch = sizeof(ROSE_Vertex2D),
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

	sampler = SDL_CreateGPUSampler(device, &sampler_create_info);

	if (!sampler)
	{
		fprintf(stderr, "SDL_CreateGPUSampler() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	depth_texture = SDL_CreateGPUTexture(device, &depth_texture_create_info);

	if (!depth_texture)
	{
		fprintf(stderr, "SDL_CreateGPUTexture() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	pipeline = SDL_CreateGPUGraphicsPipeline(device, &graphics_pipeline_create_info);

	if (!pipeline)
	{
		fprintf(stderr, "SDL_CreateGPUGraphicsPipeline() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	fclose(vertex_file);
	fclose(fragment_file);

	free(vertex_code);
	free(fragment_code);

	SDL_ReleaseGPUShader(device, vertex_shader_program);
	SDL_ReleaseGPUShader(device, fragment_shader_program);

	ROSE_Image* ascii_image = ROSE_ImageLoad("textures/ascii.png");
	ascii_texture = ROSE_SDLCreateTexture(ascii_image);
	ROSE_ImageDestroy(ascii_image);
}

void ROSE_Quit(void)
{
	// destroy resources...
}

