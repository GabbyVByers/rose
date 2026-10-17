
/*
 *   Source File [rose_text.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

#define ASCII_WIDTH 7
#define ASCII_HEIGHT 11

ROSE_Text ROSE_TextCreate(const char* string)
{
	u32 num_characters = strlen(string);
	u32 num_vertices = num_characters * 6;
	ROSE_Vertex2D* vertices = calloc(num_vertices, sizeof(ROSE_Vertex2D));

	for (u32 i = 0; i < num_characters; i++)
	{
		u32 index = i * 6;
		u32 character_index = (string[i] - 32);
		float w = (1.0f / 95.0f);
		float x = (w * (float)character_index);
		vertices[index + 0] = (ROSE_Vertex2D){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		vertices[index + 1] = (ROSE_Vertex2D){ { (float)(i + 1),  0.0f }, { (float)(x + w), 1.0f } };
		vertices[index + 2] = (ROSE_Vertex2D){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		vertices[index + 3] = (ROSE_Vertex2D){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		vertices[index + 4] = (ROSE_Vertex2D){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		vertices[index + 5] = (ROSE_Vertex2D){ { (float)(i + 0), -1.0f }, { (float)(x),     0.0f } };
	}

	ROSE_Text text = {
		.scale = 1.0,
		.xpos = 0.0,
		.ypos = 0.0,
		.color = { 1.0f, 1.0f, 1.0f, 1.0f },
		._width = ASCII_WIDTH * num_characters,
		._height = ASCII_HEIGHT,
		._count = num_vertices,
		._vertices = ROSE_SDLCreateVertexBuffer(vertices, num_vertices, sizeof(ROSE_Vertex2D)),
	};

	free(vertices);
	return text;
}

void ROSE_TextDestroy(ROSE_Text* text)
{
	if (text->_vertices)
	{
		SDL_ReleaseGPUBuffer(device, text->_vertices);
	}
	
	text->_count = 0;
	text->_vertices = NULL;
}

void ROSE_TextUpdateString(ROSE_Text* text, const char* string)
{
	assert(false && "Not Implemented!");
}

void ROSE_TextDraw(ROSE_Text* text)
{
	if (minimized)
	{
		return;
	}

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = text->_vertices,
	};

	SDL_GPUTextureSamplerBinding texture_binding = {
		.texture = ascii_texture,
		.sampler = sampler,
	};

	f64 scaled_width = (f64)text->_width * text->scale;
	f64 scaled_height = (f64)ASCII_HEIGHT * text->scale;

	f64 w = (scaled_width / (f64)screen_width) * 2.0;
	f64 h = (scaled_height / (f64)screen_height) * 2.0;
	f64 x = -1.0f + ((text->xpos / (f64)screen_width) * 2.0);
	f64 y = 1.0f - ((text->ypos / (f64)screen_height) * 2.0);

	typedef struct Uniform {
		f32 TransformMatrix[16];
		f32 TintColor[4];
	} Uniform;

	Uniform uniform = {
		.TransformMatrix = {
			(f32)w, 0, 0, 0,
			0, (f32)h, 0, 0,
			0, 0, 1, 0,
			(f32)x, (f32)y, 0, 1,
		},
		.TintColor = {
			text->color[0],
			text->color[1],
			text->color[2],
			text->color[3],
		}
	};

	SDL_BindGPUGraphicsPipeline(render_pass, sprite_pipeline);
	SDL_PushGPUVertexUniformData(command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);
	SDL_BindGPUFragmentSamplers(render_pass, 0, &texture_binding, 1);
	SDL_DrawGPUPrimitives(render_pass, (u32)text->_count, 1, 0, 0);
}

