
/*
 *   Source File [rose_gpu.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

SDL_GPUSampler* ROSE__SDLCreateSampler(void)
{
	SDL_GPUSamplerCreateInfo sampler_create_info = {
		.min_filter = SDL_GPU_FILTER_NEAREST,
		.mag_filter = SDL_GPU_FILTER_NEAREST,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
	};

	SDL_GPUSampler* sampler = SDL_CreateGPUSampler(rose__device, &sampler_create_info);
	return sampler;
}

SDL_GPUTexture* ROSE__SDLCreateTexture(ROSE_Image* image)
{
	uint32_t width = image->_width;
	uint32_t height = image->_height;
	uint32_t image_size = (width * height * (uint32_t)sizeof(ROSE_RGBA));

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

	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = image_size,
	};

	SDL_GPUTexture* texture = SDL_CreateGPUTexture(rose__device, &texture_create_info);
	SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(rose__device, &transfer_buffer_create_info);
	void* transfer_buffer_beginning = SDL_MapGPUTransferBuffer(rose__device, transfer_buffer, false);
	memcpy(transfer_buffer_beginning, image->_pixels, (size_t)image_size);
	SDL_UnmapGPUTransferBuffer(rose__device, transfer_buffer);

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

	SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(rose__device);
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
	SDL_UploadToGPUTexture(copy_pass, &texture_transfer_info, &destination_texture_region, false);
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(command_buffer);
	SDL_ReleaseGPUTransferBuffer(rose__device, transfer_buffer);
	return texture;
}

SDL_GPUBuffer* ROSE__SDLCreateVertexBuffer(void* vertices, uint32_t num_vertices, uint32_t vertex_size)
{
	SDL_GPUBufferCreateInfo buffer_create_info = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = num_vertices * vertex_size,
	};

	SDL_GPUBuffer* vertex_buffer = SDL_CreateGPUBuffer(rose__device, &buffer_create_info);

	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = num_vertices * vertex_size,
	};

	SDL_GPUTransferBuffer* transfer_buffer = SDL_CreateGPUTransferBuffer(rose__device, &transfer_buffer_create_info);
	void* transfer_buffer_beginning = SDL_MapGPUTransferBuffer(rose__device, transfer_buffer, false);
	memcpy(transfer_buffer_beginning, vertices, (size_t)num_vertices * vertex_size);
	SDL_UnmapGPUTransferBuffer(rose__device, transfer_buffer);

	SDL_GPUTransferBufferLocation source_buffer_location = {
		.transfer_buffer = transfer_buffer,
	};

	SDL_GPUBufferRegion destination_buffer_region = {
		.buffer = vertex_buffer,
		.size = num_vertices * vertex_size,
	};

	SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(rose__device);
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
	SDL_UploadToGPUBuffer(copy_pass, &source_buffer_location, &destination_buffer_region, true);
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(command_buffer);
	SDL_ReleaseGPUTransferBuffer(rose__device, transfer_buffer);
	return vertex_buffer;
}

SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipelineSprite(void)
{
	const char* vertex_path = "shaders/sprite_vertex.spv";
	const char* fragment_path = "shaders/sprite_fragment.spv";
	FILE* vertex_file = fopen(vertex_path, "rb");
	FILE* fragment_file = fopen(fragment_path, "rb");
	assert(vertex_file && "fopen() failed!");
	assert(fragment_file && "fopen() failed!");

	fseek(vertex_file, 0, SEEK_END);
	fseek(fragment_file, 0, SEEK_END);
	size_t vertex_code_size = ftell(vertex_file);
	size_t fragment_code_size = ftell(fragment_file);
	rewind(vertex_file);
	rewind(fragment_file);

	uint8_t* vertex_code = malloc(vertex_code_size);
	uint8_t* fragment_code = malloc(fragment_code_size);
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

	SDL_GPUShader* vertex_shader_program = SDL_CreateGPUShader(rose__device, &vertex_shader_create_info);
	SDL_GPUShader* fragment_shader_program = SDL_CreateGPUShader(rose__device, &fragment_shader_create_info);

	SDL_GPUVertexAttribute position_attribute = {
		.location = 0,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
		.offset = offsetof(ROSE_VertexSprite, position),
	};

	SDL_GPUVertexAttribute texcoords_attribute = {
		.location = 1,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
		.offset = offsetof(ROSE_VertexSprite, texcoords),
	};

	SDL_GPUVertexAttribute vertex_attributes[2] = {
		position_attribute,
		texcoords_attribute,
	};

	SDL_GPUVertexBufferDescription vertex_buffer_description = {
		.pitch = sizeof(ROSE_VertexSprite),
		.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
	};

	SDL_GPUColorTargetDescription color_target_description = {
		.format = SDL_GetGPUSwapchainTextureFormat(rose__device, rose__window),
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
		.target_info = {
			.color_target_descriptions = &color_target_description,
			.num_color_targets = 1,
		},
	};

	SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(rose__device, &graphics_pipeline_create_info);

	fclose(vertex_file);
	fclose(fragment_file);
	free(vertex_code);
	free(fragment_code);
	SDL_ReleaseGPUShader(rose__device, vertex_shader_program);
	SDL_ReleaseGPUShader(rose__device, fragment_shader_program);
	
	return pipeline;
}

SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipelineTriangle(void)
{
	const char* vertex_path = "shaders/triangle_vertex.spv";
	const char* fragment_path = "shaders/triangle_fragment.spv";
	FILE* vertex_file = fopen(vertex_path, "rb");
	FILE* fragment_file = fopen(fragment_path, "rb");
	assert(vertex_file && "fopen() failed!");
	assert(fragment_file && "fopen() failed!");

	fseek(vertex_file, 0, SEEK_END);
	fseek(fragment_file, 0, SEEK_END);
	size_t vertex_code_size = ftell(vertex_file);
	size_t fragment_code_size = ftell(fragment_file);
	rewind(vertex_file);
	rewind(fragment_file);

	uint8_t* vertex_code = malloc(vertex_code_size);
	uint8_t* fragment_code = malloc(fragment_code_size);
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
	};

	SDL_GPUShader* vertex_shader_program = SDL_CreateGPUShader(rose__device, &vertex_shader_create_info);
	SDL_GPUShader* fragment_shader_program = SDL_CreateGPUShader(rose__device, &fragment_shader_create_info);

	SDL_GPUVertexAttribute position_attribute = {
		.location = 0,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
		.offset = offsetof(ROSE_Vertex2D, position),
	};

	SDL_GPUVertexBufferDescription vertex_buffer_description = {
		.pitch = sizeof(ROSE_Vertex2D),
		.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
	};

	SDL_GPUColorTargetDescription color_target_description = {
		.format = SDL_GetGPUSwapchainTextureFormat(rose__device, rose__window),
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
			.vertex_attributes = &position_attribute,
			.num_vertex_attributes = 1,
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.target_info = {
			.color_target_descriptions = &color_target_description,
			.num_color_targets = 1,
		},
	};

	SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(rose__device, &graphics_pipeline_create_info);

	fclose(vertex_file);
	fclose(fragment_file);
	free(vertex_code);
	free(fragment_code);
	SDL_ReleaseGPUShader(rose__device, vertex_shader_program);
	SDL_ReleaseGPUShader(rose__device, fragment_shader_program);

	return pipeline;
}

SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipelinePoint(void)
{
	const char* vertex_path = "shaders/point_vertex.spv";
	const char* fragment_path = "shaders/point_fragment.spv";
	FILE* vertex_file = fopen(vertex_path, "rb");
	FILE* fragment_file = fopen(fragment_path, "rb");
	assert(vertex_file && "fopen() failed!");
	assert(fragment_file && "fopen() failed!");

	fseek(vertex_file, 0, SEEK_END);
	fseek(fragment_file, 0, SEEK_END);
	size_t vertex_code_size = ftell(vertex_file);
	size_t fragment_code_size = ftell(fragment_file);
	rewind(vertex_file);
	rewind(fragment_file);

	uint8_t* vertex_code = malloc(vertex_code_size);
	uint8_t* fragment_code = malloc(fragment_code_size);
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
	};

	SDL_GPUShader* vertex_shader_program = SDL_CreateGPUShader(rose__device, &vertex_shader_create_info);
	SDL_GPUShader* fragment_shader_program = SDL_CreateGPUShader(rose__device, &fragment_shader_create_info);

	SDL_GPUVertexAttribute position_attribute = {
		.location = 0,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
		.offset = offsetof(ROSE_Vertex2D, position),
	};

	SDL_GPUVertexBufferDescription vertex_buffer_description = {
		.pitch = sizeof(ROSE_Vertex2D),
		.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
	};

	SDL_GPUColorTargetDescription color_target_description = {
		.format = SDL_GetGPUSwapchainTextureFormat(rose__device, rose__window),
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
			.vertex_attributes = &position_attribute,
			.num_vertex_attributes = 1,
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_POINTLIST,
		.target_info = {
			.color_target_descriptions = &color_target_description,
			.num_color_targets = 1,
		},
	};

	SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(rose__device, &graphics_pipeline_create_info);

	fclose(vertex_file);
	fclose(fragment_file);
	free(vertex_code);
	free(fragment_code);
	SDL_ReleaseGPUShader(rose__device, vertex_shader_program);
	SDL_ReleaseGPUShader(rose__device, fragment_shader_program);

	return pipeline;
}

