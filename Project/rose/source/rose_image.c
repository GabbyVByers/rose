
/*
 *   Source File [rose_image.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

typedef struct ROSE_Image {
	u32 width;
	u32 height;
	u8* pixels;
} ROSE_Image;

ROSE_Image* ROSE_ImageCreate(u32 width, u32 height)
{
	assert(width != 0);
	assert(height != 0);

	usize count = (usize)width * height;
	u8* pixels = calloc(count, sizeof(ROSE_RGBA));
	memset(pixels, 255, count * sizeof(ROSE_RGBA));

	ROSE_Image* image = malloc(sizeof(ROSE_Image));
	image->width = width;
	image->height = height;
	image->pixels = pixels;
	return image;
}

void ROSE_ImageDestroy(ROSE_Image* image)
{
	if (!image)
	{
		return;
	}

	free(image->pixels);
	free(image);
}

ROSE_Image* ROSE_ImageLoad(const char* path)
{
	int width, height, num;
	stbi_set_flip_vertically_on_load(true);
	u8* stbi_image = stbi_load(path, &width, &height, &num, sizeof(ROSE_RGBA));

	if (!stbi_image)
	{
		fprintf(stderr, "stbi_load() Failed: %s\n", stbi_failure_reason());
		exit(EXIT_FAILURE);
	}

	usize count = (usize)width * height;
	u8* pixels = calloc(count, sizeof(ROSE_RGBA));
	memcpy(pixels, stbi_image, count * sizeof(ROSE_RGBA));
	stbi_image_free(stbi_image);

	ROSE_Image* image = malloc(sizeof(ROSE_Image));
	image->width = width;
	image->height = height;
	image->pixels = pixels;
	return image;
}

void ROSE_ImageSavePNG(ROSE_Image* image, const char* path)
{
	int width = (int)image->width;
	int height = (int)image->height;
	void* pixels = (void*)image->pixels;
	int stride = (int)image->width * 4;
	stbi_flip_vertically_on_write(true);

	if (!stbi_write_png(path, width, height, 4, pixels, stride))
	{
		fprintf(stderr, "stbi_write_png() Failed!\n");
		exit(EXIT_FAILURE);
	}
}

void ROSE_ImageResize(ROSE_Image* image, u32 width, u32 height)
{
	assert(width != 0);
	assert(height != 0);

	usize count = (usize)width * height;
	u8* pixels = calloc(count, sizeof(ROSE_RGBA));
	memset(pixels, 255, count * sizeof(ROSE_RGBA));

	usize copy_width = (width < image->width) ? width : image->width;
	usize copy_height = (height < image->height) ? height : image->height;
	usize src_offset = image->height - copy_height;
	usize dst_offset = height - copy_height;

	for (usize y = 0; y < copy_height; y++)
	{
		usize src_index = (src_offset + y) * image->width * sizeof(ROSE_RGBA);
		usize dst_index = (dst_offset + y) * width * sizeof(ROSE_RGBA);
		memcpy(pixels + dst_index, image->pixels + src_index, copy_width * sizeof(ROSE_RGBA));
	}

	free(image->pixels);
	image->width = width;
	image->height = height;
	image->pixels = pixels;
}

ROSE_RGBA ROSE_ImageGetPixelRGBA(ROSE_Image* image, u32 x, u32 y)
{
	assert(x < image->width);
	assert(y < image->height);

	usize index = ((((((usize)image->height - 1) - y) * image->width) + x) * sizeof(ROSE_RGBA));

	ROSE_RGBA color = { 0 };
	color.r = image->pixels[index + 0];
	color.g = image->pixels[index + 1];
	color.b = image->pixels[index + 2];
	color.a = image->pixels[index + 3];
	return color;
}

ROSE_Color4 ROSE_ImageGetPixelColor4(ROSE_Image* image, u32 x, u32 y)
{
	assert(x < image->width);
	assert(y < image->height);

	usize index = ((((((usize)image->height - 1) - y) * image->width) + x) * sizeof(ROSE_RGBA));

	ROSE_Color4 color = { 0 };
	color.r = ((float)(image->pixels[index + 0]) / 255.0f);
	color.g = ((float)(image->pixels[index + 1]) / 255.0f);
	color.b = ((float)(image->pixels[index + 2]) / 255.0f);
	color.a = ((float)(image->pixels[index + 3]) / 255.0f);
	return color;
}

void ROSE_ImageSetPixelRGBA(ROSE_Image* image, ROSE_RGBA color, u32 x, u32 y)
{
	assert(x < image->width);
	assert(y < image->height);

	usize index = ((((((usize)image->height - 1) - y) * image->width) + x) * sizeof(ROSE_RGBA));

	image->pixels[index + 0] = color.r;
	image->pixels[index + 1] = color.g;
	image->pixels[index + 2] = color.b;
	image->pixels[index + 3] = color.a;
}

void ROSE_ImageSetPixelColor4(ROSE_Image* image, ROSE_Color4 color, u32 x, u32 y)
{
	assert(x < image->width);
	assert(y < image->height);

	usize index = ((((((usize)image->height - 1) - y) * image->width) + x) * sizeof(ROSE_RGBA));

	image->pixels[index + 0] = (u8)(color.r * 255.0f);
	image->pixels[index + 1] = (u8)(color.g * 255.0f);
	image->pixels[index + 2] = (u8)(color.b * 255.0f);
	image->pixels[index + 3] = (u8)(color.a * 255.0f);
}

u32 ROSE_ImageWidth(ROSE_Image* image)
{
	return image->width;
}

u32 ROSE_ImageHeight(ROSE_Image* image)
{
	return image->height;
}

const u8* ROSE_ImagePixels(ROSE_Image* image)
{
	return image->pixels;
}

