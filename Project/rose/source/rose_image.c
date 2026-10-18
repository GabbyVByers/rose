
/*
 *   Source File [rose_image.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

ROSE_Image ROSE_ImageCreate(uint32_t width, uint32_t height)
{
	size_t size = (size_t)width * height * 4;
	uint8_t* pixels = NULL;

	if (size != 0) {
		pixels = malloc(size);
		memset(pixels, 255, size);
	}

	return (ROSE_Image) {
		.width = width,
		.height = height,
		.pixels = pixels,
	};
}

void ROSE_ImageDestroy(ROSE_Image* image)
{
	if (image->pixels) free(image->pixels);
	*image = (ROSE_Image) { 0 };
}

ROSE_Image ROSE_ImageLoadFromFile(const char* path)
{
	int width, height, num;
	stbi_set_flip_vertically_on_load(true);
	uint8_t* stbi_image = stbi_load(path, &width, &height, &num, 4);
	assert(stbi_image && "stbi_load() failed!");

	size_t size = (size_t)width * height * 4;
	uint8_t* pixels = malloc(size);
	memcpy(pixels, stbi_image, size);
	stbi_image_free(stbi_image);

	return (ROSE_Image) {
		.width = width,
		.height = height,
		.pixels = pixels,
	};
}

void ROSE_ImageSavePNG(ROSE_Image* image, const char* path)
{
	int width = (int)image->width;
	int height = (int)image->height;
	void* pixels = (void*)image->pixels;
	int stride = (int)image->width * 4;
	stbi_flip_vertically_on_write(true);
	bool success = stbi_write_png(path, width, height, 4, pixels, stride);
	assert(success && "stbi_write_png() failed!");
}

void ROSE_ImageResize(ROSE_Image* image, uint32_t width, uint32_t height)
{
	size_t size = (size_t)width * height * 4;
	uint8_t* pixels = NULL;
	
	if (size != 0) {
		pixels = malloc(size);
		memset(pixels, 255, size);
	}

	if (pixels && image->pixels) {
		size_t copy_width = (width < image->width) ? width : image->width;
		size_t copy_height = (height < image->height) ? height : image->height;
		size_t src_offset = image->height - copy_height;
		size_t dst_offset = height - copy_height;

		for (size_t y = 0; y < copy_height; y++) {
			size_t src_index = (src_offset + y) * image->width * 4;
			size_t dst_index = (dst_offset + y) * width * 4;
			memcpy(pixels + dst_index, image->pixels + src_index, copy_width * 4);
		}
	}

	if (image->pixels) free(image->pixels);
	*image = (ROSE_Image) {
		.width = width,
		.height = height,
		.pixels = pixels,
	};
}

ROSE_Color ROSE_ImageGetPixelColor(ROSE_Image* image, uint32_t xpos, uint32_t ypos)
{
	size_t index = ((((((size_t)image->height - 1) - ypos) * image->width) + xpos) * 4);
	float r = ((float)(image->pixels[index + 0]) / 255.0f);
	float g = ((float)(image->pixels[index + 1]) / 255.0f);
	float b = ((float)(image->pixels[index + 2]) / 255.0f);
	float a = ((float)(image->pixels[index + 3]) / 255.0f);
	return (ROSE_Color) { r, g, b, a };
}

void ROSE_ImageSetPixelColor(ROSE_Image* image, uint32_t xpos, uint32_t ypos, ROSE_Color color)
{
	size_t index = ((((((size_t)image->height - 1) - ypos) * image->width) + xpos) * 4);
	image->pixels[index + 0] = (uint8_t)(color.r * 255.0f);
	image->pixels[index + 1] = (uint8_t)(color.g * 255.0f);
	image->pixels[index + 2] = (uint8_t)(color.b * 255.0f);
	image->pixels[index + 3] = (uint8_t)(color.a * 255.0f);
}

