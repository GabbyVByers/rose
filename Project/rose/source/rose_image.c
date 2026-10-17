
/*
 *   Source File [rose_image.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

ROSE_Image ROSE_ImageCreate(u32 width, u32 height)
{
	usize size = (usize)width * height * 4;
	u8* pixels = NULL;

	if (size != 0)
	{
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
	if (image->_pixels)
	{
		free(image->_pixels);
	}

	image->_width = 0;
	image->_height = 0;
	image->_pixels = NULL;
}

void ROSE_ImageLoadFromFile(ROSE_Image* image, const char* path)
{
	if (image->_pixels)
	{
		free(image->_pixels);
	}

	image->_width = 0;
	image->_height = 0;
	image->_pixels = NULL;

	int width, height, num;
	stbi_set_flip_vertically_on_load(true);
	u8* stbi_image = stbi_load(path, &width, &height, &num, 4);
	assert(stbi_image && "stbi_load() Failed!");

	usize size = (usize)width * height * 4;
	u8* pixels = malloc(size);
	memcpy(pixels, stbi_image, size);
	stbi_image_free(stbi_image);

	image->_width = width;
	image->_height = height;
	image->_pixels = pixels;
}

void ROSE_ImageSavePNG(ROSE_Image* image, const char* path)
{
	int width = (int)image->_width;
	int height = (int)image->_height;
	void* pixels = (void*)image->_pixels;
	int stride = (int)image->_width * 4;
	stbi_flip_vertically_on_write(true);
	bool success = stbi_write_png(path, width, height, 4, pixels, stride);
	assert(success && "stbi_write_png() Failed!");
}

void ROSE_ImageResize(ROSE_Image* image, u32 width, u32 height)
{
	usize size = (usize)width * height * 4;
	u8* pixels = NULL;
	
	if (size != 0)
	{
		pixels = malloc(size);
		memset(pixels, 255, size);
	}

	if (pixels && image->_pixels)
	{
		usize copy_width = (width < image->_width) ? width : image->_width;
		usize copy_height = (height < image->_height) ? height : image->_height;
		usize src_offset = image->_height - copy_height;
		usize dst_offset = height - copy_height;

		for (usize y = 0; y < copy_height; y++)
		{
			usize src_index = (src_offset + y) * image->_width * 4;
			usize dst_index = (dst_offset + y) * width * 4;
			memcpy(pixels + dst_index, image->_pixels + src_index, copy_width * 4);
		}
	}

	if (image->_pixels)
	{
		free(image->_pixels);
	}

	image->_width = width;
	image->_height = height;
	image->_pixels = pixels;
}

void ROSE_ImageGetPixelColor(ROSE_Image* image, u32 xpos, u32 ypos, float color[4])
{
	usize index = ((((((usize)image->_height - 1) - ypos) * image->_width) + xpos) * 4);
	color[0] = ((float)(image->_pixels[index + 0]) / 255.0f);
	color[1] = ((float)(image->_pixels[index + 1]) / 255.0f);
	color[2] = ((float)(image->_pixels[index + 2]) / 255.0f);
	color[3] = ((float)(image->_pixels[index + 3]) / 255.0f);
	return color;
}

void ROSE_ImageSetPixelColor(ROSE_Image* image, u32 xpos, u32 ypos, float color[4])
{
	usize index = ((((((usize)image->_height - 1) - ypos) * image->_width) + xpos) * 4);
	image->_pixels[index + 0] = (u8)(color[0] * 255.0f);
	image->_pixels[index + 1] = (u8)(color[1] * 255.0f);
	image->_pixels[index + 2] = (u8)(color[2] * 255.0f);
	image->_pixels[index + 3] = (u8)(color[3] * 255.0f);
}

