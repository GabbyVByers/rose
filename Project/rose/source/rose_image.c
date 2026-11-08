
/*
 *   Source [rose_image.c]
 */

#include "rose.h"

ROSE_Image ROSE_ImageCreate(uInt32 width, uInt32 height)
{
	const bool nonzero =
		(width != 0) &&
		(height != 0);

	uInt32* pixels = NULL;
	if (nonzero) {
		uInt32 count = width * height;
		pixels = calloc(count, sizeof(uInt32));
		memset(pixels, (uInt8)255, count * sizeof(uInt32));
	}

	ROSE_Image image = { 0 };
	image._width = width;
	image._height = height;
	image._pixels = pixels;
	return image;
}

ROSE_Image ROSE_ImageCreateFromPerlin(ROSE_Perlin* perlin)
{
	uInt32 width = perlin->_width;
	uInt32 height = perlin->_height;
	ROSE_Image image = ROSE_ImageCreate(width, height);

	for (uInt32 y = 0; y < height; y++) {
		for (uInt32 x = 0; x < width; x++) {
			float intensity = perlin->_noise[(y * width) + x];
			if (intensity < 0.0f) intensity = 0.0f;
			if (intensity > 1.0f) intensity = 1.0f;
			uInt8 c = (uInt8)((intensity * 255.0f) + 0.5f);
			uInt32 color = ROSE_COLOR(c, c, c, c);
			ROSE_ImageSetPixel(&image, x, y, color);
		}
	}

	return image;
}

void ROSE_ImageDestroy(ROSE_Image* image)
{
	if (image->_pixels) {
		free(image->_pixels);
	}
	
	image->_width = 0;
	image->_height = 0;
	image->_pixels = NULL;
}

ROSE_Image ROSE_ImageLoad(const char* path)
{
	uInt32 width = 0;
	uInt32 height = 0;
	uInt32 pixels = NULL;
	int retcode = png_decode(path, &pixels, &width, &height);

	ROSE_Image image = { 0 };
	image._width = width;
	image._height = height;
	image._pixels = pixels;

	if (retcode != 0) {
		fprintf(stderr, "png_decode() failed! [path: %s] [retcode: %s]", png_error(retcode));
	}
	
	return image;
}

void ROSE_ImageSave(ROSE_Image* image, const char* path)
{
	uInt32 width = image->_width;
	uInt32 height = image->_height;
	uInt32* pixels = image->_pixels;
	int retcode = png_encode(path, pixels, width, height);

	if (retcode != 0) {
		fprintf(stderr, "png_encode() failed! [path: %s] [retcode: %s]", png_error(retcode));
	}
}

void ROSE_ImageResize(ROSE_Image* image, uInt32 width, uInt32 height)
{
	uInt32 count = width * height;
	uInt32* pixels = NULL;
	
	if (count != 0) {
		pixels = calloc(count, sizeof(uInt32));
		memset(pixels, 255, count * sizeof(uInt32));
	}

	if (pixels && image->_pixels) {
		uInt32 copy_width = (width < image->_width) ? width : image->_width;
		uInt32 copy_height = (height < image->_height) ? height : image->_height;
		uInt32 src_offset = image->_height - copy_height;
		uInt32 dst_offset = height - copy_height;

		for (uInt32 y = 0; y < copy_height; y++) {
			uInt32 src_index = (src_offset + y) * image->_width;
			uInt32 dst_index = (dst_offset + y) * width;
			uInt32 copy_size = copy_width * sizeof(uInt32);
			memcpy(pixels + dst_index, image->_pixels + src_index, copy_size);
		}
	}

	if (image->_pixels) {
		free(image->_pixels);
	}
	
	image->_width = width;
	image->_height = height;
	image->_pixels = pixels;
}

void ROSE_ImageFillColor(ROSE_Image* image, uInt32 color)
{
	for (uInt32 y = 0; y < image->_height; y++) {
		for (uInt32 x = 0; x < image->_width; x++) {
			ROSE_ImageSetPixel(image, x, y, color);
		}
	}
}

uInt32 ROSE_ImageGetPixel(ROSE_Image* image, uInt32 xpos, uInt32 ypos)
{
	assert(xpos < image->_width);
	assert(ypos < image->_height);
	uInt32 index = (ypos * image->_width) + xpos;
	uInt32 color = image->_pixels[index];
	return color;
}

void ROSE_ImageSetPixel(ROSE_Image* image, uInt32 xpos, uInt32 ypos, uInt32 color)
{
	assert(xpos < image->_width);
	assert(ypos < image->_height);
	uInt32 index = (ypos * image->_width) + xpos;
	image->_pixels[index] = color;
}

