
/*
 *   Source File [rose_text.c]
 */

#include "rose.h"

/*
* 128 * 6 = 768
* 128 characters @
* 6 verts per character
*/ #define BUFFER_SIZE 768

extern SDL_GPUDevice* device;

ROSE_Text* ROSE_TextCreate(const char* string) {
	ROSE_Text* text = malloc(sizeof(ROSE_Text));
	assert(text);

	usize num_characters = strlen(string);
	usize num_vertices = num_characters * 6;
	assert(num_vertices < BUFFER_SIZE);
	
	static ROSE_Vertex vertices[BUFFER_SIZE];
	for (usize i = 0; i < num_characters; i++) {
		usize index = i * 6;
		char character_index = (string[i] - 32); // todo: map oor chars to '?'
		float w = (1.0f / 95.0f);
		float x = (w * (float)character_index);
		vertices[index + 0] = (ROSE_Vertex){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		vertices[index + 1] = (ROSE_Vertex){ { (float)(i + 1),  0.0f }, { (float)(x + w), 1.0f } };
		vertices[index + 2] = (ROSE_Vertex){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		vertices[index + 3] = (ROSE_Vertex){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		vertices[index + 4] = (ROSE_Vertex){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		vertices[index + 5] = (ROSE_Vertex){ { (float)(i + 0), -1.0f }, { (float)(x),     0.0f } };
	}

	text->width = 7;
	text->height = 11;
	text->buffer = ROSE_INTERNAL_CreateVertexBuffer(vertices, num_vertices);
	text->num_vertices = num_vertices;
	return text;
}

void ROSE_TextDestroy(ROSE_Text* text) {
	if (!text) { return; }
	SDL_ReleaseGPUBuffer(device, text->buffer);
	free(text);
}

