
/*
 *   Source File [rose_text.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

ROSE_Text ROSE_TextCreate(const char* string)
{
	uint32_t num_characters = strlen(string);
	uint32_t num_vertices = num_characters * 6;
	ROSE_VertexSprite* vertices = calloc(num_vertices, sizeof(ROSE_VertexSprite));

	for (uint32_t i = 0; i < num_characters; i++) {
		uint32_t index = i * 6;
		uint32_t character_index = (string[i] - 32);
		float w = (1.0f / 95.0f);
		float x = (w * (float)character_index);
		vertices[index + 0] = (ROSE_VertexSprite){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		vertices[index + 1] = (ROSE_VertexSprite){ { (float)(i + 1),  0.0f }, { (float)(x + w), 1.0f } };
		vertices[index + 2] = (ROSE_VertexSprite){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		vertices[index + 3] = (ROSE_VertexSprite){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		vertices[index + 4] = (ROSE_VertexSprite){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		vertices[index + 5] = (ROSE_VertexSprite){ { (float)(i + 0), -1.0f }, { (float)(x),     0.0f } };
	}

	ROSE_Text text = (ROSE_Text) {
		.scale = 1.0,
		.xpos = 0.0,
		.ypos = 0.0,
		.color = (ROSE_Color) { 1.0f, 1.0f, 1.0f, 1.0f },
		.num_characters = num_characters,
		.num_vertices = num_vertices,
		.vertex_buffer = ROSE_SDLCreateVertexBuffer(vertices, num_vertices, sizeof(ROSE_VertexSprite)),
	};

	free(vertices);
	return text;
}

void ROSE_TextDestroy(ROSE_Text* text)
{
	if (text->vertex_buffer) SDL_ReleaseGPUBuffer(device, text->vertex_buffer);
	*text = (ROSE_Text){ 0 };
}

void ROSE_TextDraw(ROSE_Text* text)
{
	if (minimized)
		return;

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = text->vertex_buffer,
	};

	SDL_GPUTextureSamplerBinding texture_binding = {
		.texture = ascii_texture,
		.sampler = sampler,
	};

	double scaled_width = (double)ROSE_ASCII_WIDTH * text->scale;
	double scaled_height = (double)ROSE_ASCII_HEIGHT * text->scale;

	double ww = (scaled_width / (double)screen_width) * 2.0;
	double hh = (scaled_height / (double)screen_height) * 2.0;
	double xx = -1.0f + ((text->xpos / (double)screen_width) * 2.0);
	double yy = 1.0f - ((text->ypos / (double)screen_height) * 2.0);

	float w = (float)ww;
	float h = (float)hh;
	float x = (float)xx;
	float y = (float)yy;

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

	SDL_BindGPUGraphicsPipeline(render_pass, sprite_pipeline);
	SDL_PushGPUVertexUniformData(command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);
	SDL_BindGPUFragmentSamplers(render_pass, 0, &texture_binding, 1);
	SDL_DrawGPUPrimitives(render_pass, text->num_vertices, 1, 0, 0);
}

