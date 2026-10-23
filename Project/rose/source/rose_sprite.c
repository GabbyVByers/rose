
/*
 *   Source File [rose_sprite.c]
 */

#define ROSE_EXTERN
#include "rose.h"

ROSE_Sprite ROSE_SpriteCreate(ROSE_Image* image)
{
	return (ROSE_Sprite) {
		.scale = 1,
		.xpos = 0,
		.ypos = 0,
		._width = image->_width,
		._height = image->_height,
		.color = ROSE_COLOR_WHITE,
		._texture = ROSE__SDLCreateTexture(image),
	};
}

ROSE_Sprite ROSE_SpriteCreateEmpty(uint32_t width, uint32_t height)
{
	return (ROSE_Sprite) {
		.scale = 1,
		.xpos = 0,
		.ypos = 0,
		._width = width,
		._height = height,
		.color = ROSE_COLOR_WHITE,
		._texture = NULL,
	};
}

ROSE_Sprite ROSE_SpriteLoadFromFile(const char* path)
{
	ROSE_Image image = ROSE_ImageLoadFromFile(path);
	ROSE_Sprite sprite = ROSE_SpriteCreate(&image);
	ROSE_ImageDestroy(&image);
	return sprite;
}

void ROSE_SpriteDestroy(ROSE_Sprite* sprite)
{
	if (sprite) {
		if (sprite->_texture) SDL_ReleaseGPUTexture(rose__device, sprite->_texture);
		*sprite = (ROSE_Sprite) { 0 };
	}
}

void ROSE_SpriteUpdateTexture(ROSE_Sprite* sprite, ROSE_Image* image)
{
	bool reuse_texture = (sprite->_texture)
		&& (sprite->_width == image->_width)
		&& (sprite->_height == image->_height);
	
	if (reuse_texture) {
		ROSE__SDLUploadImageToTexture(sprite->_texture, image);
		return;
	}

	if (sprite->_texture)
		SDL_ReleaseGPUTexture(rose__device, sprite->_texture);
	
	sprite->_texture = ROSE__SDLCreateTexture(image);
	sprite->_width = image->_width;
	sprite->_height = image->_height;
}

void ROSE_SpriteDraw(ROSE_Sprite* sprite)
{
	if (rose_screen.minimized) return;
	if (!rose__render_pass) abort();

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = rose__sprite_quad_buffer,
	};

	SDL_GPUTextureSamplerBinding texture_binding = {
		.texture = (sprite->_texture) ? (sprite->_texture) : (rose__dummy_texture),
		.sampler = rose__sampler,
	};

	float scaled_width = (float)sprite->_width * sprite->scale;
	float scaled_height = (float)sprite->_height * sprite->scale;

	float w = (scaled_width / (float)rose_screen.width) * 2.0f;
	float h = (scaled_height / (float)rose_screen.height) * 2.0f;
	float x = -1.0f + (((float)sprite->xpos / (float)rose_screen.width) * 2.0f);
	float y = 1.0f - (((float)sprite->ypos / (float)rose_screen.height) * 2.0f);

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

	SDL_BindGPUGraphicsPipeline(rose__render_pass, rose__sprite_pipeline);
	SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(rose__render_pass, 0, &buffer_binding, 1);
	SDL_BindGPUFragmentSamplers(rose__render_pass, 0, &texture_binding, 1);
	SDL_DrawGPUPrimitives(rose__render_pass, 6, 1, 0, 0);
}

