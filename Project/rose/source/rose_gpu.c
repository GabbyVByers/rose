
/*
 *   Source File [rose_gpu.c]
 */

#define ROSE_EXTERN
#include "rose.h"

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
	SDL_GPUTextureCreateInfo texture_create_info = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
		.width = image->_width,
		.height = image->_height,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.sample_count = SDL_GPU_SAMPLECOUNT_1,
	};

	SDL_GPUTexture* texture = SDL_CreateGPUTexture(rose__device, &texture_create_info);
	ROSE__SDLUploadImageToTexture(texture, image);
	return texture;
}

void ROSE__SDLUploadImageToTexture(SDL_GPUTexture* texture, ROSE_Image* image)
{
	uint32_t width = image->_width;
	uint32_t height = image->_height;
	uint32_t image_size = (width * height * (uint32_t)sizeof(ROSE_RGBA));

	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = image_size,
	};

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
	SDL_UploadToGPUTexture(copy_pass, &texture_transfer_info, &destination_texture_region, true);
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(command_buffer);
	SDL_ReleaseGPUTransferBuffer(rose__device, transfer_buffer);
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

SDL_GPUShader* ROSE__SDLLoadShader(const char* path, SDL_GPUShaderStage stage, uint32_t uniforms, uint32_t samplers)
{
	size_t code_size = 0;
	void* code = SDL_LoadFile(path, &code_size);
	if (!code) {
		fprintf(stderr, "SDL_LoadFile() failed!\n");
		abort();
	}

	SDL_GPUShaderCreateInfo shader_create_info = {
		.code_size = code_size,
		.code = code,
		.entrypoint = "main",
		.format = SDL_GPU_SHADERFORMAT_SPIRV,
		.stage = stage,
		.num_uniform_buffers = uniforms,
		.num_samplers = samplers,
	};

	SDL_GPUShader* shader = SDL_CreateGPUShader(rose__device, &shader_create_info);
	SDL_free(code);
	return shader;
}

SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipeline(const char* vert, const char* frag, uint32_t samplers, SDL_GPUVertexInputState input, SDL_GPUPrimitiveType primitive)
{
	SDL_GPUShader* vertex_shader_program = ROSE__SDLLoadShader(vert, SDL_GPU_SHADERSTAGE_VERTEX, 1, 0);
	SDL_GPUShader* fragment_shader_program = ROSE__SDLLoadShader(frag, SDL_GPU_SHADERSTAGE_FRAGMENT, 0, samplers);

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
		.vertex_input_state = input,
		.primitive_type = primitive,
		.target_info = {
			.color_target_descriptions = &color_target_description,
			.num_color_targets = 1,
		},
	};

	SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(rose__device, &graphics_pipeline_create_info);
	SDL_ReleaseGPUShader(rose__device, vertex_shader_program);
	SDL_ReleaseGPUShader(rose__device, fragment_shader_program);
	return pipeline;
}

SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipelineSprite(void)
{
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

	SDL_GPUVertexInputState vertex_input_state = {
		.vertex_buffer_descriptions = &vertex_buffer_description,
		.num_vertex_buffers = 1,
		.vertex_attributes = vertex_attributes,
		.num_vertex_attributes = sizeof(vertex_attributes) / sizeof(vertex_attributes[0]),
	};

	return ROSE__SDLCreateGraphicsPipeline(
		"shaders/sprite_vertex.spv",
		"shaders/sprite_fragment.spv",
		1, vertex_input_state,
		SDL_GPU_PRIMITIVETYPE_TRIANGLELIST
	);
}

SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipelineTriangles(void)
{
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

	SDL_GPUVertexInputState vertex_input_state = {
		.vertex_buffer_descriptions = &vertex_buffer_description,
		.num_vertex_buffers = 1,
		.vertex_attributes = &position_attribute,
		.num_vertex_attributes = 1,
	};

	return ROSE__SDLCreateGraphicsPipeline(
		"shaders/triangles_vertex.spv",
		"shaders/triangles_fragment.spv",
		0, vertex_input_state,
		SDL_GPU_PRIMITIVETYPE_TRIANGLELIST
	);
}

SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipelinePoints(void)
{
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

	SDL_GPUVertexInputState vertex_input_state = {
		.vertex_buffer_descriptions = &vertex_buffer_description,
		.num_vertex_buffers = 1,
		.vertex_attributes = &position_attribute,
		.num_vertex_attributes = 1,
	};

	return ROSE__SDLCreateGraphicsPipeline(
		"shaders/points_vertex.spv",
		"shaders/points_fragment.spv",
		0, vertex_input_state,
		SDL_GPU_PRIMITIVETYPE_POINTLIST
	);
}

SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipelineLines(void)
{
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

	SDL_GPUVertexInputState vertex_input_state = {
		.vertex_buffer_descriptions = &vertex_buffer_description,
		.num_vertex_buffers = 1,
		.vertex_attributes = &position_attribute,
		.num_vertex_attributes = 1,
	};

	return ROSE__SDLCreateGraphicsPipeline(
		"shaders/triangles_vertex.spv",
		"shaders/triangles_fragment.spv",
		0, vertex_input_state,
		SDL_GPU_PRIMITIVETYPE_LINELIST
	);
}

