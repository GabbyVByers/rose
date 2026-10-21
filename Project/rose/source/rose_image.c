
/*
 *   Source File [rose_image.c]
 */

#define ROSE_EXTERN
#include "rose.h"

ROSE_Image ROSE_ImageCreate(uint32_t width, uint32_t height)
{
	size_t size = (size_t)width * height * 4;
	uint8_t* pixels = NULL;

	if (size != 0) {
		pixels = malloc(size);
		memset(pixels, 255, size);
	}

	return (ROSE_Image) {
		._width = width,
		._height = height,
		._pixels = pixels,
	};
}

void ROSE_ImageDestroy(ROSE_Image* image)
{
	if (image) {
		if (image->_pixels) free(image->_pixels);
		*image = (ROSE_Image) { 0 };
	}
}

ROSE_Image ROSE_ImageLoadFromFile(const char* path)
{
	int width, height, num;
	stbi_set_flip_vertically_on_load(true);
	uint8_t* stbi_image = stbi_load(path, &width, &height, &num, 4);
	if (!stbi_image) {
		fprintf(stderr, "stbi_load() failed!\n");
		abort();
	}

	size_t size = (size_t)width * height * 4;
	uint8_t* pixels = malloc(size);
	memcpy(pixels, stbi_image, size);
	stbi_image_free(stbi_image);

	return (ROSE_Image) {
		._width = width,
		._height = height,
		._pixels = pixels,
	};
}

void ROSE_ImageSavePNG(ROSE_Image* image, const char* path)
{
	int width = (int)image->_width;
	int height = (int)image->_height;
	void* pixels = (void*)image->_pixels;
	int stride = (int)image->_width * 4;
	stbi_flip_vertically_on_write(true);
	bool success = stbi_write_png(path, width, height, 4, pixels, stride);
	if (!success) {
		fprintf(stderr, "stbi_write_png() failed!\n");
		abort();
	}
}

void ROSE_ImageResize(ROSE_Image* image, uint32_t width, uint32_t height)
{
	size_t size = (size_t)width * height * 4;
	uint8_t* pixels = NULL;
	
	if (size != 0) {
		pixels = malloc(size);
		memset(pixels, 255, size);
	}

	if (pixels && image->_pixels) {
		size_t copy_width = (width < image->_width) ? width : image->_width;
		size_t copy_height = (height < image->_height) ? height : image->_height;
		size_t src_offset = image->_height - copy_height;
		size_t dst_offset = height - copy_height;

		for (size_t y = 0; y < copy_height; y++) {
			size_t src_index = (src_offset + y) * image->_width * 4;
			size_t dst_index = (dst_offset + y) * width * 4;
			memcpy(pixels + dst_index, image->_pixels + src_index, copy_width * 4);
		}
	}

	if (image->_pixels) free(image->_pixels);
	*image = (ROSE_Image) {
		._width = width,
		._height = height,
		._pixels = pixels,
	};
}

ROSE_Color ROSE_ImageGetPixelColor(ROSE_Image* image, uint32_t xpos, uint32_t ypos)
{
	assert(xpos < image->_width);
	assert(ypos < image->_height);
	size_t index = ((((((size_t)image->_height - 1) - ypos) * image->_width) + xpos) * 4);
	float r = ((float)(image->_pixels[index + 0]) / 255.0f);
	float g = ((float)(image->_pixels[index + 1]) / 255.0f);
	float b = ((float)(image->_pixels[index + 2]) / 255.0f);
	float a = ((float)(image->_pixels[index + 3]) / 255.0f);
	return (ROSE_Color) { r, g, b, a };
}

ROSE_RGBA ROSE_ImageGetPixelRGBA(ROSE_Image* image, uint32_t xpos, uint32_t ypos)
{
	assert(xpos < image->_width);
	assert(ypos < image->_height);
	size_t index = ((((((size_t)image->_height - 1) - ypos) * image->_width) + xpos) * 4);
	uint8_t r = image->_pixels[index + 0];
	uint8_t g = image->_pixels[index + 1];
	uint8_t b = image->_pixels[index + 2];
	uint8_t a = image->_pixels[index + 3];
	return (ROSE_RGBA) { r, g, b, a };
}

void ROSE_ImageSetPixelColor(ROSE_Image* image, uint32_t xpos, uint32_t ypos, ROSE_Color color)
{
	assert(xpos < image->_width);
	assert(ypos < image->_height);
	size_t index = ((((((size_t)image->_height - 1) - ypos) * image->_width) + xpos) * 4);
	image->_pixels[index + 0] = (uint8_t)((color.r * 255.0f) + 0.5f);
	image->_pixels[index + 1] = (uint8_t)((color.g * 255.0f) + 0.5f);
	image->_pixels[index + 2] = (uint8_t)((color.b * 255.0f) + 0.5f);
	image->_pixels[index + 3] = (uint8_t)((color.a * 255.0f) + 0.5f);
}

void ROSE_ImageSetPixelRGBA(ROSE_Image* image, uint32_t xpos, uint32_t ypos, ROSE_RGBA color)
{
	assert(xpos < image->_width);
	assert(ypos < image->_height);
	size_t index = ((((((size_t)image->_height - 1) - ypos) * image->_width) + xpos) * 4);
	image->_pixels[index + 0] = color.r;
	image->_pixels[index + 1] = color.g;
	image->_pixels[index + 2] = color.b;
	image->_pixels[index + 3] = color.a;
}

