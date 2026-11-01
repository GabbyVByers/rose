
/*
 *   Source File [rose_text.c]
 */

#define ROSE_EXTERN
#include "rose.h"

#define MAX_STRING_LENGTH 512

ROSE_Text ROSE_TextCreate(const char* format, ...)
{
	char string[MAX_STRING_LENGTH] = { '\0' };
	va_list arguments = { 0 };
	va_start(arguments, format);
	vsnprintf(string, sizeof(string), format, arguments);
	va_end(arguments);

	ROSE_Text text = ROSE_TextCreateEmpty();
	ROSE_TextUpdateString(&text, "%s", string);
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
		._string = NULL,
		._vertex_buffer = NULL,
		._transfer_buffer = NULL,
	};
}

void ROSE_TextDestroy(ROSE_Text* text)
{
	if (text) {
		if (text->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, text->_vertex_buffer);
		if (text->_transfer_buffer) SDL_ReleaseGPUTransferBuffer(rose__device, text->_transfer_buffer);
		if (text->_string) free(text->_string);
		*text = (ROSE_Text){ 0 };
	}
}

void ROSE_TextUpdateString(ROSE_Text* text, const char* format, ...)
{
	char string[MAX_STRING_LENGTH] = { '\0' };
	va_list arguments = { 0 };
	va_start(arguments, format);
	vsnprintf(string, sizeof(string), format, arguments);
	va_end(arguments);

	uint32_t num_characters = (uint32_t)strlen(string);
	bool unchanged = (num_characters == text->_num_characters)
		&& ((num_characters == 0) || (memcmp(text->_string, string, num_characters) == 0));

	if (unchanged)
		return;

	ROSE__TextReserveCharacterCapacity(text, num_characters);
	text->_num_characters = num_characters;
	text->_num_vertices = num_characters * 6;

	if (num_characters == 0)
		return;

	memcpy(text->_string, string, num_characters);
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

	SDL_UploadToGPUBuffer(ROSE__SDLAcquireCopyPass(), &source_buffer_location, &destination_buffer_region, true);
}

void ROSE_TextDraw(ROSE_Text* text)
{
	if (rose_screen.minimized) return;
	if (!rose__render_pass) ROSE_PANIC("ROSE_TextDraw() called outside of a frame!");
	if (text->_num_vertices == 0) return;

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

	ROSE__SDLBindGraphicsPipeline(rose__sprite_pipeline);
	SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	ROSE__SDLBindVertexBuffer(text->_vertex_buffer);
	ROSE__SDLBindTexture(rose__ascii_texture);
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
	if (text->_string) free(text->_string);

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
	text->_string = malloc(capacity);
	text->_character_capacity = capacity;
}

