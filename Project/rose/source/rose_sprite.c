
/*
 *   Source File [rose_sprite.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

typedef struct ROSE_Sprite {
	u32 width;
	u32 height;
	SDL_GPUBuffer* buffer;
	SDL_GPUTexture* texture;
} ROSE_Sprite;

ROSE_Sprite* ROSE_SpriteCreate(ROSE_Image* image)
{
	ROSE_Vertex2D vertices[] = {
		{ { 0, 0 }, { 0, 1 } },
		{ { 1, 0 }, { 1, 1 } },
		{ { 1,-1 }, { 1, 0 } },
		{ { 0, 0 }, { 0, 1 } },
		{ { 1,-1 }, { 1, 0 } },
		{ { 0,-1 }, { 0, 0 } },
	};

	ROSE_Sprite* sprite = malloc(sizeof(ROSE_Sprite));
	u32 count = sizeof(vertices) / sizeof(vertices[0]);
	sprite->width = ROSE_ImageWidth(image);
	sprite->height = ROSE_ImageHeight(image);
	sprite->buffer = ROSE_SDLCreateVertexBuffer(vertices, count, sizeof(ROSE_Vertex2D));
	sprite->texture = ROSE_SDLCreateTexture(image);
	return sprite;
}

void ROSE_SpriteDestroy(ROSE_Sprite* sprite)
{
	if (!sprite)
	{
		return;
	}

	SDL_ReleaseGPUTexture(device, sprite->texture);
	SDL_ReleaseGPUBuffer(device, sprite->buffer);
	free(sprite);
}

void ROSE_SpriteDraw(ROSE_Sprite* sprite, i32 px, i32 py, float scale, ROSE_Color4 color)
{
	if (minimized)
	{
		return;
	}

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = sprite->buffer,
	};

	SDL_GPUTextureSamplerBinding texture_binding = {
		.texture = sprite->texture,
		.sampler = sampler,
	};

	float scaled_width = (float)sprite->width * scale;
	float scaled_height = (float)sprite->height * scale;

	float w = (scaled_width / (float)screen_width) * 2.0f;
	float h = (scaled_height / (float)screen_height) * 2.0f;
	float x = -1.0f + (((float)px / (float)screen_width) * 2.0f);
	float y = 1.0f - (((float)py / (float)screen_height) * 2.0f);

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
			color.r,
			color.g,
			color.b,
			color.a,
		}
	};

	SDL_PushGPUVertexUniformData(command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);
	SDL_BindGPUFragmentSamplers(render_pass, 0, &texture_binding, 1);
	SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);
}

u32 ROSE_SpriteWidth(ROSE_Sprite* sprite)
{
	return sprite->width;
}

u32 ROSE_SpriteHeight(ROSE_Sprite* sprite)
{
	return sprite->height;
}

