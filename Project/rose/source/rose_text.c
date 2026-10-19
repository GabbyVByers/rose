
/*
 *   Source File [rose_text.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

ROSE_Text ROSE_TextCreate(const char* text_string)
{
	uint32_t num_characters = strlen(text_string);
	uint32_t num_vertices = num_characters * 6;
	ROSE_VertexSprite* cpu_vertices = malloc(num_vertices * sizeof(ROSE_VertexSprite));

	for (uint32_t i = 0; i < num_characters; i++) {
		uint32_t index = i * 6;
		uint32_t character_index = (text_string[i] - 32);
		float w = (1.0f / 95.0f);
		float x = (w * (float)character_index);
		cpu_vertices[index + 0] = (ROSE_VertexSprite){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		cpu_vertices[index + 1] = (ROSE_VertexSprite){ { (float)(i + 1),  0.0f }, { (float)(x + w), 1.0f } };
		cpu_vertices[index + 2] = (ROSE_VertexSprite){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		cpu_vertices[index + 3] = (ROSE_VertexSprite){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		cpu_vertices[index + 4] = (ROSE_VertexSprite){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		cpu_vertices[index + 5] = (ROSE_VertexSprite){ { (float)(i + 0), -1.0f }, { (float)(x),     0.0f } };
	}

	ROSE_Text text = (ROSE_Text) {
		.scale = 1.0,
		.xpos = 0.0,
		.ypos = 0.0,
		.color = ROSE_COLOR_WHITE,
		._num_characters = num_characters,
		._num_vertices = num_vertices,
		._cpu_vertices = cpu_vertices,
		._vertex_buffer = ROSE__SDLCreateVertexBuffer(cpu_vertices, num_vertices, sizeof(ROSE_VertexSprite)),
	};

	return text;
}

void ROSE_TextDestroy(ROSE_Text* text)
{
	if (text) {
		if (text->_cpu_vertices) free(text->_cpu_vertices);
		if (text->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, text->_vertex_buffer);
		*text = (ROSE_Text){ 0 };
	}
}

void ROSE_TextUpdateString(ROSE_Text* text, const char* new_string)
{
	uint32_t num_characters = strlen(new_string);
	if (text->_num_characters != num_characters) {
		uint32_t num_vertices = num_characters * 6;
		ROSE_VertexSprite* cpu_vertices = malloc(num_vertices * sizeof(ROSE_VertexSprite));

		for (uint32_t i = 0; i < num_characters; i++) {
			uint32_t index = i * 6;
			uint32_t character_index = (new_string[i] - 32);
			float w = (1.0f / 95.0f);
			float x = (w * (float)character_index);
			cpu_vertices[index + 0] = (ROSE_VertexSprite){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
			cpu_vertices[index + 1] = (ROSE_VertexSprite){ { (float)(i + 1),  0.0f }, { (float)(x + w), 1.0f } };
			cpu_vertices[index + 2] = (ROSE_VertexSprite){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
			cpu_vertices[index + 3] = (ROSE_VertexSprite){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
			cpu_vertices[index + 4] = (ROSE_VertexSprite){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
			cpu_vertices[index + 5] = (ROSE_VertexSprite){ { (float)(i + 0), -1.0f }, { (float)(x),     0.0f } };
		}

		if (text->_cpu_vertices) free(text->_cpu_vertices);
		if (text->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, text->_vertex_buffer);
		text->_num_characters = num_characters;
		text->_num_vertices = num_vertices;
		text->_cpu_vertices = cpu_vertices;
		text->_vertex_buffer = ROSE__SDLCreateVertexBuffer(cpu_vertices, num_vertices, sizeof(ROSE_VertexSprite));
		return;
	}

	for (uint32_t i = 0; i < text->_num_characters; i++) {
		uint32_t index = i * 6;
		uint32_t character_index = (new_string[i] - 32);
		float w = (1.0f / 95.0f);
		float x = (w * (float)character_index);
		text->_cpu_vertices[index + 0] = (ROSE_VertexSprite){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		text->_cpu_vertices[index + 1] = (ROSE_VertexSprite){ { (float)(i + 1),  0.0f }, { (float)(x + w), 1.0f } };
		text->_cpu_vertices[index + 2] = (ROSE_VertexSprite){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		text->_cpu_vertices[index + 3] = (ROSE_VertexSprite){ { (float)(i + 0),  0.0f }, { (float)(x),     1.0f } };
		text->_cpu_vertices[index + 4] = (ROSE_VertexSprite){ { (float)(i + 1), -1.0f }, { (float)(x + w), 0.0f } };
		text->_cpu_vertices[index + 5] = (ROSE_VertexSprite){ { (float)(i + 0), -1.0f }, { (float)(x),     0.0f } };
	}

	if (text->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, text->_vertex_buffer);
	text->_vertex_buffer = ROSE__SDLCreateVertexBuffer(text->_cpu_vertices, text->_num_vertices, sizeof(ROSE_VertexSprite));
}

void ROSE_TextDraw(ROSE_Text* text)
{
	if (rose_screen.minimized)
		return;

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = text->_vertex_buffer,
	};

	SDL_GPUTextureSamplerBinding texture_binding = {
		.texture = rose__ascii_texture,
		.sampler = rose__sampler,
	};

	double scaled_width = (double)ROSE_ASCII_WIDTH * text->scale;
	double scaled_height = (double)ROSE_ASCII_HEIGHT * text->scale;

	double ww = (scaled_width / (double)rose_screen.width) * 2.0;
	double hh = (scaled_height / (double)rose_screen.height) * 2.0;
	double xx = -1.0f + ((text->xpos / (double)rose_screen.width) * 2.0);
	double yy = 1.0f - ((text->ypos / (double)rose_screen.height) * 2.0);

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

	SDL_BindGPUGraphicsPipeline(rose__render_pass, rose__sprite_pipeline);
	SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(rose__render_pass, 0, &buffer_binding, 1);
	SDL_BindGPUFragmentSamplers(rose__render_pass, 0, &texture_binding, 1);
	SDL_DrawGPUPrimitives(rose__render_pass, text->_num_vertices, 1, 0, 0);
}

