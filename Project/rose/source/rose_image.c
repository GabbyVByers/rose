
/*
 *   Source File [rose_image.c]
 */

#include "rose.h"

ROSE_Image* ROSE_ImageCreate(i32 width, i32 height) {
	usize buffer_size = (usize)width * (usize)height * 4;
	u8* pixels = malloc(buffer_size);
	memset(pixels, 255, buffer_size);

	ROSE_Image* image = malloc(sizeof(ROSE_Image));
	image->width = width;
	image->height = height;
	image->pixels = pixels;
	return image;
}

ROSE_Image* ROSE_ImageLoadFromFile(const char* path) {
	int width, height, n;
	stbi_set_flip_vertically_on_load(TRUE);
	u8* stbi_image = stbi_load(path, &width, &height, &n, 4);
	
	usize buffer_size = (usize)width * (usize)height * 4;
	u8* pixels = malloc(buffer_size);
	memcpy(pixels, stbi_image, buffer_size);
	stbi_image_free(stbi_image);
	
	ROSE_Image* image = malloc(sizeof(ROSE_Image));
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

void ROSE_ImageSaveAsPNG(ROSE_Image* image, const char* name) {
	i32 width = image->width;
	i32 height = image->height;
	u8* pixels = image->pixels;
	i32 stride = image->width * 4;
	stbi_flip_vertically_on_write(TRUE);
	i32 status = stbi_write_png(name, width, height, 4, pixels, stride);
}

void ROSE_ImageResize(ROSE_Image* image, i32 width, i32 height) {
	usize new_buffer_size = (usize)width * (usize)height * 4;
	u8* new_pixels = malloc(new_buffer_size);
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

void ROSE_ImagePutPixel(ROSE_Image* image, i32 x, i32 y, ROSE_Color color) {
	assert((x < image->width) && (y < image->height));
	usize index = ((((((usize)image->height - 1) - y) * (usize)image->width) + (usize)x) * 4);
	image->pixels[index + 0] = (u8)(color.r * 255.0f);
	image->pixels[index + 1] = (u8)(color.g * 255.0f);
	image->pixels[index + 2] = (u8)(color.b * 255.0f);
	image->pixels[index + 3] = (u8)(color.a * 255.0f);
}

ROSE_Color ROSE_ImageGetPixel(ROSE_Image* image, i32 x, i32 y) {
	assert((x < image->width) && (y < image->height));
	usize index = ((((((usize)image->height - 1) - y) * (usize)image->width) + (usize)x) * 4);
	return (ROSE_Color) {
		.r = ((float)(image->pixels[index + 0]) / 255.0f),
		.g = ((float)(image->pixels[index + 1]) / 255.0f),
		.b = ((float)(image->pixels[index + 2]) / 255.0f),
		.a = ((float)(image->pixels[index + 3]) / 255.0f),
	};
}

