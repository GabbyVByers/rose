
/*
 *   Source File [rose_gpu.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

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

