
/*
 *   Source File [rose_gpu.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

SDL_GPUTexture* ROSE_SDLCreateDepthTexture(void)
{
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

	if (!texture)
	{
		fprintf(stderr, "SDL_CreateGPUTexture() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	return texture;
}

SDL_GPUSampler* ROSE_SDLCreateSampler(void)
{
	SDL_GPUSamplerCreateInfo sampler_create_info = {
		.min_filter = SDL_GPU_FILTER_NEAREST,
		.mag_filter = SDL_GPU_FILTER_NEAREST,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
	};

	SDL_GPUSampler* sampler = SDL_CreateGPUSampler(device, &sampler_create_info);

	if (!sampler)
	{
		fprintf(stderr, "SDL_CreateGPUSampler() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	return sampler;
}

SDL_GPUTexture* ROSE_SDLCreateTexture(ROSE_Image* image)
{
	u32 width = ROSE_ImageWidth(image);
	u32 height = ROSE_ImageHeight(image);
	u32 image_size = (width * height * (u32)sizeof(ROSE_RGBA));

	SDL_GPUTextureCreateInfo texture_create_info = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
		.width = width,
		.height = height,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.sample_count = SDL_GPU_SAMPLECOUNT_1,
	};

	SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &texture_create_info);

	if (!texture)
	{
		fprintf(stderr, "SDL_CreateGPUTexture() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = image_size,
	};

	SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_buffer_create_info);

	if (!transfer_buffer)
	{
		fprintf(stderr, "SDL_CreateGPUTransferBuffer() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	void* transfer_buffer_beginning = SDL_MapGPUTransferBuffer(device, transfer_buffer, false);

	if (!transfer_buffer_beginning)
	{
		fprintf(stderr, "SDL_MapGPUTransferBuffer() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	memcpy(transfer_buffer_beginning, ROSE_ImagePixels(image), (usize)image_size);
	SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

	SDL_GPUTextureTransferInfo texture_transfer_info = {
		.transfer_buffer = transfer_buffer,
		.pixels_per_row = width,
		.rows_per_layer = height,
	};

	SDL_GPUTextureRegion destination_texture_region = {
		.texture = texture,
		.w = width,
		.h = height,
		.d = 1,
	};

	SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);

	if (!command_buffer)
	{
		fprintf(stderr, "SDL_AcquireGPUCommandBuffer() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

	if (!copy_pass)
	{
		fprintf(stderr, "SDL_BeginGPUCopyPass() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_UploadToGPUTexture(copy_pass, &texture_transfer_info, &destination_texture_region, false);
	SDL_EndGPUCopyPass(copy_pass);

	if (!SDL_SubmitGPUCommandBuffer(command_buffer))
	{
		fprintf(stderr, "SDL_SubmitGPUCommandBuffer() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
	return texture;
}

SDL_GPUBuffer* ROSE_SDLCreateVertexBuffer(void* vertices, u32 count, u32 stride)
{
	assert(count != 0);
	assert(stride != 0);

	u32 buffer_size = count * stride;

	SDL_GPUBufferCreateInfo buffer_create_info = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = buffer_size,
	};

	SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(device, &buffer_create_info);

	if (!buffer)
	{
		fprintf(stderr, "SDL_CreateGPUBuffer() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = buffer_size,
	};

	SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(device, &transfer_buffer_create_info);

	if (!transfer_buffer)
	{
		fprintf(stderr, "SDL_CreateGPUTransferBuffer() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	void* transfer_buffer_beginning = SDL_MapGPUTransferBuffer(device, transfer_buffer, false);

	if (!transfer_buffer_beginning)
	{
		fprintf(stderr, "SDL_MapGPUTransferBuffer() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	memcpy(transfer_buffer_beginning, vertices, buffer_size);
	SDL_UnmapGPUTransferBuffer(device, transfer_buffer);

	SDL_GPUTransferBufferLocation source_buffer_location = {
		.transfer_buffer = transfer_buffer,
	};

	SDL_GPUBufferRegion destination_buffer_region = {
		.buffer = buffer,
		.size = buffer_size,
	};

	SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(device);

	if (!command_buffer)
	{
		fprintf(stderr, "SDL_AcquireGPUCommandBuffer() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

	if (!copy_pass)
	{
		fprintf(stderr, "SDL_BeginGPUCopyPass() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_UploadToGPUBuffer(copy_pass, &source_buffer_location, &destination_buffer_region, true);
	SDL_EndGPUCopyPass(copy_pass);

	if (!SDL_SubmitGPUCommandBuffer(command_buffer))
	{
		fprintf(stderr, "SDL_SubmitGPUCommandBuffer() Failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_ReleaseGPUTransferBuffer(device, transfer_buffer);
	return buffer;
}

SDL_GPUGraphicsPipeline* ROSE_SDLCreateGraphicsPipelineSprite(void)
{
	const char* vertex_path = "shaders/sprite_vertex.spv";
	const char* fragment_path = "shaders/sprite_fragment.spv";
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

	SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &graphics_pipeline_create_info);

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
	return pipeline;
}

SDL_GPUGraphicsPipeline* ROSE_SDLCreateGraphicsPipelineMesh(void)
{
	const char* vertex_path = "shaders/mesh_vertex.spv";
	const char* fragment_path = "shaders/mesh_fragment.spv";
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
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
		.offset = offsetof(ROSE_Vertex3D, position),
	};

	SDL_GPUVertexAttribute color_attribute = {
		.location = 1,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
		.offset = offsetof(ROSE_Vertex3D, color),
	};

	SDL_GPUVertexAttribute texcoords_attribute = {
		.location = 2,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
		.offset = offsetof(ROSE_Vertex3D, texcoords),
	};

	SDL_GPUVertexAttribute vertex_attributes[] = {
		position_attribute,
		color_attribute,
		texcoords_attribute,
	};

	SDL_GPUVertexBufferDescription vertex_buffer_description = {
		.pitch = sizeof(ROSE_Vertex3D),
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

	SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &graphics_pipeline_create_info);

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
	return pipeline;
}

