
/*
 *   Source File [rose_sprite.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

ROSE_Sprite ROSE_SpriteCreate(ROSE_Image* image)
{
	ROSE_VertexSprite vertices[] = {
		{ { 0, 0 }, { 0, 1 } },
		{ { 1, 0 }, { 1, 1 } },
		{ { 1,-1 }, { 1, 0 } },
		{ { 0, 0 }, { 0, 1 } },
		{ { 1,-1 }, { 1, 0 } },
		{ { 0,-1 }, { 0, 0 } },
	};

	return (ROSE_Sprite) {
		.scale = 1.0,
		.xpos = 0,
		.ypos = 0,
		.width = image->width,
		.height = image->height,
		.color = { 1, 1, 1, 1 },
		.vertex_buffer = ROSE_SDLCreateVertexBuffer(vertices, sizeof(vertices) / sizeof(vertices[0]), sizeof(ROSE_VertexSprite)),
		.texture = ROSE_SDLCreateTexture(image),
	};
}

void ROSE_SpriteDestroy(ROSE_Sprite* sprite)
{
	if (sprite->texture) SDL_ReleaseGPUTexture(device, sprite->texture);
	if (sprite->vertex_buffer) SDL_ReleaseGPUBuffer(device, sprite->vertex_buffer);
	*sprite = (ROSE_Sprite){ 0 };
}

void ROSE_SpriteDraw(ROSE_Sprite* sprite)
{
	if (minimized)
		return;

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = sprite->vertex_buffer,
	};

	SDL_GPUTextureSamplerBinding texture_binding = {
		.texture = sprite->texture,
		.sampler = sampler,
	};

	double scaled_width = (double)sprite->width * sprite->scale;
	double scaled_height = (double)sprite->height * sprite->scale;

	double ww = (scaled_width / (double)screen_width) * 2.0f;
	double hh = (scaled_height / (double)screen_height) * 2.0f;
	double xx = -1.0f + (((double)sprite->xpos / (double)screen_width) * 2.0f);
	double yy = 1.0f - (((double)sprite->ypos / (double)screen_height) * 2.0f);

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
			sprite->color.r,
			sprite->color.g,
			sprite->color.b,
			sprite->color.a,
		}
	};

	SDL_BindGPUGraphicsPipeline(render_pass, sprite_pipeline);
	SDL_PushGPUVertexUniformData(command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);
	SDL_BindGPUFragmentSamplers(render_pass, 0, &texture_binding, 1);
	SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);
}

