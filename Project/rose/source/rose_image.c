
/*
 *   Source File [rose_image.c]
 */

#include "rose.h"

ROSE_Image* ROSE_ImageCreate(i32 width, i32 height) {
	assert(width > 0);
	assert(height > 0);

	ROSE_Image* image = malloc(sizeof(ROSE_Image));
	assert(image);

	usize buffer_size = (usize)width * (usize)height * 4;
	u8* pixels = malloc(buffer_size);
	assert(pixels);
	memset(pixels, 255, buffer_size);
	
	image->width = width;
	image->height = height;
	image->pixels = pixels;
	return image;
}

void ROSE_ImageDestroy(ROSE_Image* image) {
	if (!image) { return; }
	free(image->pixels);
	free(image);
}

ROSE_Image* ROSE_ImageLoadFromFile(const char* path) {
	assert(path);

	ROSE_Image* image = malloc(sizeof(ROSE_Image));
	assert(image);
	
	int width, height, n;
	stbi_set_flip_vertically_on_load(TRUE);
	u8* stbi_image = stbi_load(path, &width, &height, &n, 4);
	assert(stbi_image);
	
	usize buffer_size = (usize)width * (usize)height * 4;
	u8* pixels = malloc(buffer_size);
	assert(pixels);
	
	memcpy(pixels, stbi_image, buffer_size);
	stbi_image_free(stbi_image);
	
	image->width = width;
	image->height = height;
	image->pixels = pixels;
	return image;
}

void ROSE_ImageSaveAsPNG(ROSE_Image* image, const char* name) {
	assert(image);
	assert(name);
	
	i32 width = image->width;
	i32 height = image->height;
	i32 stride = image->width * 4;
	
	stbi_flip_vertically_on_write(TRUE);
	i32 success = stbi_write_png(name, width, height, 4, image->pixels, stride);
	assert(success);
}

void ROSE_ImageResize(ROSE_Image* image, i32 width, i32 height) {
	assert(image);
	assert(width > 0);
	assert(height > 0);
	
	usize new_buffer_size = (usize)width * (usize)height * 4;
	u8* new_pixels = malloc(new_buffer_size);
	assert(new_pixels);
	memset(new_pixels, 255, new_buffer_size);

	i32 copy_width = (width < image->width) ? width : image->width;
	i32 copy_height = (height < image->height) ? height : image->height;
	
	i32 src_offset = image->height - copy_height;
	i32 dst_offset = height - copy_height;
	
	for (i32 y = 0; y < copy_height; y++) {
		usize src_index = (usize)(src_offset + y) * (usize)image->width * 4;
		usize dst_index = (usize)(dst_offset + y) * (usize)width * 4;
		memcpy(new_pixels + dst_index, image->pixels + src_index, (usize)copy_width * 4);
	}

	free(image->pixels);
	image->width = width;
	image->height = height;
	image->pixels = new_pixels;
}

ROSE_Color ROSE_ImageGetPixel(ROSE_Image* image, i32 x, i32 y) {
	assert(image);
	assert(x < image->width);
	assert(y < image->height);
	
	usize index = ((((((usize)image->height - 1) - y) * (usize)image->width) + (usize)x) * 4);
	u8 r = image->pixels[index + 0];
	u8 g = image->pixels[index + 1];
	u8 b = image->pixels[index + 2];
	u8 a = image->pixels[index + 3];
	
	return (ROSE_Color) {
		.r = ((float)r / 255.0f),
		.g = ((float)g / 255.0f),
		.b = ((float)b / 255.0f),
		.a = ((float)a / 255.0f),
	};
}

void ROSE_ImageSetPixel(ROSE_Image* image, i32 x, i32 y, ROSE_Color color) {
	assert(image);
	assert(x < image->width);
	assert(y < image->height);
	
	usize index = ((((((usize)image->height - 1) - y) * (usize)image->width) + (usize)x) * 4);
	image->pixels[index + 0] = (u8)(color.r * 255.0f);
	image->pixels[index + 1] = (u8)(color.g * 255.0f);
	image->pixels[index + 2] = (u8)(color.b * 255.0f);
	image->pixels[index + 3] = (u8)(color.a * 255.0f);
}

