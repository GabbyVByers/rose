
/*
 *   Source File [rose_text.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

ROSE_Text* ROSE_TextCreate(const char* string)
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

	static const u32 ascii_width = 7;
	static const u32 ascii_height = 11;

	ROSE_Text* text = calloc(1, sizeof(ROSE_Text));
	text->width = ascii_width;
	text->height = ascii_height;
	text->full_width = num_characters * ascii_width;
	text->buffer = ROSE_SDLCreateVertexBuffer(vertices, num_vertices, sizeof(ROSE_Vertex2D));
	text->num_vertices = num_vertices;
	free(vertices);
	return text;
}

void ROSE_TextDestroy(ROSE_Text* text)
{
	if (!text)
	{
		return;
	}

	SDL_ReleaseGPUBuffer(device, text->buffer);
	free(text);
}

void ROSE_TextUpdateString(ROSE_Text* text, const char* string)
{
	assert(false && "NOT IMPLEMENTED");
}

void ROSE_TextDraw(ROSE_Text* text, i32 px, i32 py, f32 scale, ROSE_Color color)
{
	if (minimized)
	{
		return;
	}

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = text->buffer,
	};

	SDL_GPUTextureSamplerBinding texture_binding = {
		.texture = ascii_texture,
		.sampler = sampler,
	};

	f32 scaled_width = (f32)text->width * scale;
	f32 scaled_height = (f32)text->height * scale;

	f32 w = (scaled_width / (f32)ROSE_SCREEN_WIDTH) * 2.0f;
	f32 h = (scaled_height / (f32)ROSE_SCREEN_HEIGHT) * 2.0f;
	f32 x = -1.0f + (((f32)px / (f32)ROSE_SCREEN_WIDTH) * 2.0f);
	f32 y = 1.0f - (((f32)py / (f32)ROSE_SCREEN_HEIGHT) * 2.0f);

	typedef struct Uniform {
		f32 TransformMatrix[16];
		f32 TintColor[4];
	} Uniform;

	Uniform uniform = {
		.TransformMatrix = {
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, 1, 0,
			x, y, 0, 1,
		},
		.TintColor = {
			color.r,
			color.g,
			color.b,
			color.a,
		}
	};

	SDL_BindGPUGraphicsPipeline(render_pass, sprite_pipeline);
	SDL_PushGPUVertexUniformData(command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);
	SDL_BindGPUFragmentSamplers(render_pass, 0, &texture_binding, 1);
	SDL_DrawGPUPrimitives(render_pass, (u32)text->num_vertices, 1, 0, 0);
}

u32 ROSE_TextWidth(ROSE_Text* text)
{
	return text->full_width;
}

u32 ROSE_TextHeight(ROSE_Text* text)
{
	return text->height;
}

