
/*
 *   Source File [rose_text.c]
 */

#define ROSE_EXTERN
#include "rose.h"

ROSE_Text ROSE_TextCreate(const char* string)
{
	ROSE_Text text = ROSE_TextCreateEmpty();
	ROSE_TextUpdateString(&text, string);
	return text;
}

ROSE_Text ROSE_TextCreateEmpty(void)
{
	return (ROSE_Text) {
		.scale = 1,
		.xpos = 0,
		.ypos = 0,
		.color = ROSE_COLOR_WHITE,
		._num_characters = 0,
		._num_vertices = 0,
		._character_capacity = 0,
		._vertex_buffer = NULL,
		._transfer_buffer = NULL,
	};
}

void ROSE_TextDestroy(ROSE_Text* text)
{
	if (text) {
		if (text->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, text->_vertex_buffer);
		if (text->_transfer_buffer) SDL_ReleaseGPUTransferBuffer(rose__device, text->_transfer_buffer);
		*text = (ROSE_Text){ 0 };
	}
}

void ROSE_TextUpdateString(ROSE_Text* text, const char* string)
{
	uint32_t num_characters = (uint32_t)strlen(string);
	ROSE__TextReserveCharacterCapacity(text, num_characters);
	text->_num_characters = num_characters;
	text->_num_vertices = num_characters * 6;

	if (num_characters == 0)
		return;

	ROSE_VertexSprite* vertices = SDL_MapGPUTransferBuffer(rose__device, text->_transfer_buffer, true);
	ROSE__TextWriteCharactersToVertices(vertices, string, num_characters);
	SDL_UnmapGPUTransferBuffer(rose__device, text->_transfer_buffer);

	SDL_GPUTransferBufferLocation source_buffer_location = {
		.transfer_buffer = text->_transfer_buffer,
	};

	SDL_GPUBufferRegion destination_buffer_region = {
		.buffer = text->_vertex_buffer,
		.size = text->_num_vertices * (uint32_t)sizeof(ROSE_VertexSprite),
	};

	SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(rose__device);
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
	SDL_UploadToGPUBuffer(copy_pass, &source_buffer_location, &destination_buffer_region, true);
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(command_buffer);
}

void ROSE_TextDraw(ROSE_Text* text)
{
	if (rose_screen.minimized) return;
	if (!rose__render_pass) abort();

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = text->_vertex_buffer,
	};

	SDL_GPUTextureSamplerBinding texture_binding = {
		.texture = rose__ascii_texture,
		.sampler = rose__sampler,
	};

	float scaled_width = (float)ROSE_ASCII_WIDTH * (float)text->scale;
	float scaled_height = (float)ROSE_ASCII_HEIGHT * (float)text->scale;

	float w = (scaled_width / (float)rose_screen.width) * 2.0f;
	float h = (scaled_height / (float)rose_screen.height) * 2.0f;
	float x = -1.0f + (((float)text->xpos / (float)rose_screen.width) * 2.0f);
	float y = 1.0f - (((float)text->ypos / (float)rose_screen.height) * 2.0f);

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
			text->color.r,
			text->color.g,
			text->color.b,
			text->color.a,
		}
	};

	SDL_BindGPUGraphicsPipeline(rose__render_pass, rose__sprite_pipeline);
	SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(rose__render_pass, 0, &buffer_binding, 1);
	SDL_BindGPUFragmentSamplers(rose__render_pass, 0, &texture_binding, 1);
	SDL_DrawGPUPrimitives(rose__render_pass, text->_num_vertices, 1, 0, 0);
}

void ROSE__TextWriteCharactersToVertices(ROSE_VertexSprite* vertices, const char* string, uint32_t num_characters)
{
	const float w = (1.0f / 95.0f);
	for (uint32_t i = 0; i < num_characters; i++) {
		uint32_t index = i * 6;
		unsigned char c = (unsigned char)string[i];
		uint32_t character_index = ((c >= 32) && (c <= 126)) ? (c - 32) : ('?' - 32);
		float x = (w * (float)character_index);
		vertices[index + 0] = (ROSE_VertexSprite){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		vertices[index + 1] = (ROSE_VertexSprite){ { (float)(i + 1),  0.0f }, { (float)(x + w), 1.0f } };
		vertices[index + 2] = (ROSE_VertexSprite){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		vertices[index + 3] = (ROSE_VertexSprite){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		vertices[index + 4] = (ROSE_VertexSprite){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		vertices[index + 5] = (ROSE_VertexSprite){ { (float)(i + 0), -1.0f }, { (float)(x),     0.0f } };
	}
}

void ROSE__TextReserveCharacterCapacity(ROSE_Text* text, uint32_t num_characters)
{
	if (num_characters <= text->_character_capacity)
		return;

	const uint32_t min_capacity = 32;
	uint32_t capacity = text->_character_capacity;
	if (capacity == 0) {
		capacity = min_capacity;
	}

	while (capacity < num_characters)
		capacity *= 2;

	uint32_t vertex_buffer_size = capacity * 6 * (uint32_t)sizeof(ROSE_VertexSprite);

	if (text->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, text->_vertex_buffer);
	if (text->_transfer_buffer) SDL_ReleaseGPUTransferBuffer(rose__device, text->_transfer_buffer);

	SDL_GPUBufferCreateInfo buffer_create_info = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = vertex_buffer_size,
	};

	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = vertex_buffer_size,
	};

	text->_vertex_buffer = SDL_CreateGPUBuffer(rose__device, &buffer_create_info);
	text->_transfer_buffer = SDL_CreateGPUTransferBuffer(rose__device, &transfer_buffer_create_info);
	text->_character_capacity = capacity;
}

