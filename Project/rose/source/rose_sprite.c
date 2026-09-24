
/*
 *   Source File [rose_sprite.c]
 */

#include "rose.h"

extern SDL_GPUDevice* device;

ROSE_Sprite* ROSE_SpriteCreate(const char* path) {
	ROSE_Sprite* sprite = (ROSE_Sprite*)malloc(sizeof(ROSE_Sprite));
	ROSE_Image* image = NULL;
	if (path) { image = ROSE_ImageLoadFromFile(path); }
	else { image = ROSE_ImageCreate(10, 10); }

	sprite->width = image->width;
	sprite->height = image->height;
	ROSE_Vertex vertices[6] = {
		{ { 0, 0 }, { 0, 1 } },
		{ { 1, 0 }, { 1, 1 } },
		{ { 1,-1 }, { 1, 0 } },
		{ { 0, 0 }, { 0, 1 } },
		{ { 1,-1 }, { 1, 0 } },
		{ { 0,-1 }, { 0, 0 } },
	};

	sprite->buffer = ROSE_INTERNAL_CreateVertexBuffer(vertices, sizeof(vertices) / sizeof(ROSE_Vertex));
	sprite->texture = ROSE_INTERNAL_CreateRenderTexture(image->width, image->height);
	ROSE_INTERNAL_UploadImageToRenderTexture(image, sprite->texture);
	return sprite;
}

void ROSE_SpriteDestroy(ROSE_Sprite* sprite) {
	if (!sprite) { return; }
	SDL_ReleaseGPUBuffer(device, sprite->buffer);
	SDL_ReleaseGPUTexture(device, sprite->texture);
	free(sprite);
}

void ROSE_SpriteUploadTexture(ROSE_Sprite* sprite, ROSE_Image* image) {
	sprite->width = image->width;
	sprite->height = image->height;
	SDL_ReleaseGPUTexture(device, sprite->texture);
	sprite->texture = ROSE_INTERNAL_CreateRenderTexture(image->width, image->height);
	ROSE_INTERNAL_UploadImageToRenderTexture(image, sprite->texture);
}

