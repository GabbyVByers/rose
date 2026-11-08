
/*
 *   Source [rose_text.c]
 */

#include "rose.h"

#define MAX_STRING_LENGTH 512
#define VERTS_PER_QUAD 6

ROSE_Text ROSE_TextCreate(const char* format, ...)
{
	char string[MAX_STRING_LENGTH] = { '\0' };
	va_list args = { 0 };
	va_start(args, format);
	vsnprintf(string, sizeof(string), format, args);
	va_end(args);

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
		._character_count = 0,
		._vertex_count = 0,
		._character_capacity = 0,
		._string = NULL,
		._vertex_buffer = NULL,
		._transfer_buffer = NULL,
	};
}

void ROSE_TextDestroy(ROSE_Text* text)
{
	if (text->_vertex_buffer) {
		SDL_ReleaseGPUBuffer(rose__device, text->_vertex_buffer);
	}
	
	if (text->_transfer_buffer) {
		SDL_ReleaseGPUTransferBuffer(rose__device, text->_transfer_buffer);
	}

	if (text->_string) {
		free(text->_string);
	}

	*text = (ROSE_Text){ 0 };
}

void ROSE_TextUpdateString(ROSE_Text* text, const char* format, ...)
{
	char string[MAX_STRING_LENGTH] = { '\0' };
	va_list args = { 0 };
	va_start(args, format);
	vsnprintf(string, sizeof(string), format, args);
	va_end(args);

	uInt32 character_count = (uInt32)strlen(string);
	bool unchanged = (character_count == text->_character_count)
		&& ((character_count == 0) || (memcmp(text->_string, string, character_count) == 0));
	if (unchanged) {
		return;
	}

	ROSE__TextReserveCharacterCapacity(text, character_count);
	text->_character_count = character_count;
	text->_vertex_count = character_count * 6;
	if (character_count == 0) {
		return;
	}

	memcpy(text->_string, string, character_count);
	ROSE_VertexSprite* vertices = SDL_MapGPUTransferBuffer(rose__device, text->_transfer_buffer, true);
	ROSE__TextWriteCharactersToVertices(vertices, string, character_count);
	SDL_UnmapGPUTransferBuffer(rose__device, text->_transfer_buffer);

	SDL_GPUTransferBufferLocation source_buffer_location = {
		.transfer_buffer = text->_transfer_buffer,
	};

	SDL_GPUBufferRegion destination_buffer_region = {
		.buffer = text->_vertex_buffer,
		.size = text->_vertex_count * (uInt32)sizeof(ROSE_VertexSprite),
	};

	SDL_UploadToGPUBuffer(ROSE__SDLAcquireCopyPass(), &source_buffer_location, &destination_buffer_region, true);
}

void ROSE_TextDraw(ROSE_Text* text)
{
	if (rose_screen.minimized) {
		return;
	}

	if (!rose__render_pass) {
		ROSE_PANIC("ROSE_TextDraw() called outside of a frame!");
	}

	if (text->_vertex_count == 0) {
		return;
	}

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

	const uInt32 color = text->color;
	float r = ((float)((uInt8)(color << 24))) / 255.0f;
	float g = ((float)((uInt8)(color << 16))) / 255.0f;
	float b = ((float)((uInt8)(color << 8)))  / 255.0f;
	float a = ((float)((uInt8)(color)))       / 255.0f;

	Uniform uniform = {
		.TransformMatrix = {
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, 1, 0,
			x, y, 0, 1,
		},
		.TintColor = {
			r, g, b, a,
		}
	};

	ROSE__SDLBindGraphicsPipeline(rose__sprite_pipeline);
	SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	ROSE__SDLBindVertexBuffer(text->_vertex_buffer);
	ROSE__SDLBindTexture(rose__ascii_texture);
	SDL_DrawGPUPrimitives(rose__render_pass, text->_vertex_count, 1, 0, 0);
}

void ROSE__TextWriteCharactersToVertices(ROSE_VertexSprite* vertices, const char* string, uInt32 num_characters)
{
	const float w = (1.0f / (float)ROSE_ASCII_GLYPHS);
	for (uInt32 i = 0; i < num_characters; i++) {
		uInt32 index = i * 6;
		unsigned char c = (unsigned char)string[i];
		uInt32 character_index = ((c >= 32) && (c <= 126)) ? (c - 32) : ('?' - 32);
		float x = (w * (float)character_index);
		vertices[index + 0] = (ROSE_VertexSprite){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		vertices[index + 1] = (ROSE_VertexSprite){ { (float)(i + 1),  0.0f }, { (float)(x + w), 1.0f } };
		vertices[index + 2] = (ROSE_VertexSprite){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		vertices[index + 3] = (ROSE_VertexSprite){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		vertices[index + 4] = (ROSE_VertexSprite){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		vertices[index + 5] = (ROSE_VertexSprite){ { (float)(i + 0), -1.0f }, { (float)(x),     0.0f } };
	}
}

void ROSE__TextReserveCharacterCapacity(ROSE_Text* text, uInt32 num_characters)
{
	if (num_characters <= text->_character_capacity) {
		return;
	}

	const uInt32 min_capacity = 32;
	uInt32 capacity = text->_character_capacity;
	if (capacity == 0) {
		capacity = min_capacity;
	}

	while (capacity < num_characters) {
		capacity *= 2;
	}

	uInt32 vertex_buffer_size = capacity * VERTS_PER_QUAD * (uInt32)sizeof(ROSE_VertexSprite);

	if (text->_vertex_buffer) {
		SDL_ReleaseGPUBuffer(rose__device, text->_vertex_buffer);
	}

	if (text->_transfer_buffer) {
		SDL_ReleaseGPUTransferBuffer(rose__device, text->_transfer_buffer);
	}

	if (text->_string) {
		free(text->_string);
	}

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

