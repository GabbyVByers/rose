
/*
 *   Source [rose_sprite.c]
 */

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
		._transfer_buffer = NULL,
	};
}

ROSE_Sprite ROSE_SpriteCreateEmpty(uInt32 width, uInt32 height)
{
	return (ROSE_Sprite) {
		.scale = 1,
		.xpos = 0,
		.ypos = 0,
		._width = width,
		._height = height,
		.color = ROSE_COLOR_WHITE,
		._texture = NULL,
		._transfer_buffer = NULL,
	};
}

ROSE_Sprite ROSE_SpriteLoadFromFile(const char* path)
{
	ROSE_Image image = ROSE_ImageLoad(path);
	ROSE_Sprite sprite = ROSE_SpriteCreate(&image);
	ROSE_ImageDestroy(&image);
	return sprite;
}

void ROSE_SpriteDestroy(ROSE_Sprite* sprite)
{
	if (sprite) {
		if (sprite->_texture) SDL_ReleaseGPUTexture(rose__device, sprite->_texture);
		if (sprite->_transfer_buffer) SDL_ReleaseGPUTransferBuffer(rose__device, sprite->_transfer_buffer);
		*sprite = (ROSE_Sprite) { 0 };
	}
}

void ROSE_SpriteUpdateTexture(ROSE_Sprite* sprite, ROSE_Image* image)
{
	bool reuse_texture = (sprite->_texture)
		&& (sprite->_width == image->_width)
		&& (sprite->_height == image->_height);
	
	if (reuse_texture) {
		ROSE__SDLUploadImageToTexture(sprite->_texture, &sprite->_transfer_buffer, image);
		return;
	}

	if (sprite->_texture)
		SDL_ReleaseGPUTexture(rose__device, sprite->_texture);
	if (sprite->_transfer_buffer)
		SDL_ReleaseGPUTransferBuffer(rose__device, sprite->_transfer_buffer);

	sprite->_texture = ROSE__SDLCreateTexture(image);
	sprite->_transfer_buffer = NULL;
	sprite->_width = image->_width;
	sprite->_height = image->_height;
}

void ROSE_SpriteDraw(ROSE_Sprite* sprite)
{
	if (rose_screen.minimized) return;
	if (!rose__render_pass) ROSE_PANIC("ROSE_SpriteDraw() called outside of a frame!");

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

	const uInt32 color = sprite->color;
	float r = ((float)((uInt8)(color >> 24))) / 255.0f;
	float g = ((float)((uInt8)(color >> 16))) / 255.0f;
	float b = ((float)((uInt8)(color >> 8))) / 255.0f;
	float a = ((float)((uInt8)(color))) / 255.0f;

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
	ROSE__SDLBindVertexBuffer(rose__sprite_quad_buffer);
	ROSE__SDLBindTexture((sprite->_texture) ? (sprite->_texture) : (rose__dummy_texture));
	SDL_DrawGPUPrimitives(rose__render_pass, 6, 1, 0, 0);
}

ROSE_Sprite ROSE_SpriteCreateGUIBox(uInt32 width, uInt32 height, bool normal)
{
	const bool zero = (width != 0) && (height != 0);
	if (zero) {
		return ROSE_SpriteCreateEmpty(width, height);
	}

	ROSE_Image image = ROSE_ImageCreate(width, height);

	const uInt32 color        = ROSE_COLOR(100, 100, 100, 255);
	const uInt32 color_top    = (normal) ? ROSE_COLOR(135, 135, 135, 255) : ROSE_COLOR( 60,  60,  60, 255);
	const uInt32 color_bottom = (normal) ? ROSE_COLOR( 70,  70,  70, 255) : ROSE_COLOR(135, 135, 135, 255);

	for (uInt32 y = 0; y < height; y++) {
		for (uInt32 x = 0; x < width; x++) {
			ROSE_ImageSetPixel(&image, x, y, color);
		}
	}

	for (uInt32 y = 0; y < height; y++) {
		ROSE_ImageSetPixel(&image, (width - 1), y, color_bottom);
		ROSE_ImageSetPixel(&image, 0, y, color_top);
	}

	for (uInt32 x = 0; x < width; x++) {
		ROSE_ImageSetPixel(&image, x, (height - 1), color_bottom);
		ROSE_ImageSetPixel(&image, x, 0, color_top);
	}

	ROSE_Sprite sprite = ROSE_SpriteCreate(&image);
	ROSE_ImageDestroy(&image);
	return sprite;
}

