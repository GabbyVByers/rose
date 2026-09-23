
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

static float lerp(float left, float right, float weight) {
	return (left * (1.0f - weight)) + (right * weight);
}

static float smoothstep(float value) {
	static const float pi = 3.1415927f;
	return 0.5f + (sin((value * pi) - (pi * 0.5f)) * 0.5f);
}

ROSE_Image* ROSE_ImagePerlinNoise(i32 width, i32 divisions) {
	typedef struct {
		float px, py;
		float dx, dy;
	} Arrow;

	assert(width > 0);
	assert(divisions > 0);
	usize arrow_buffer_width = (usize)divisions + 3;
	usize num_arrows = arrow_buffer_width * arrow_buffer_width;
	Arrow* arrow_buffer = malloc(num_arrows * sizeof(Arrow));
	assert(arrow_buffer);

	for (usize i = 0; i < arrow_buffer_width; i++) {
		for (usize j = 0; j < arrow_buffer_width; j++) {
			float angle = ((float)rand() / RAND_MAX) * 6.28318535f;
			Arrow arrow;
			arrow.px = (float)i;
			arrow.py = (float)j;
			arrow.dx = cos(angle);
			arrow.dy = sin(angle);
			arrow_buffer[(j * arrow_buffer_width) + i] = arrow;
		}
	}

	usize stride = ((usize)width / (usize)divisions) + 1;
	usize num_pixels = (usize)width * (usize)width;
	float* intensity_buffer = malloc(num_pixels * sizeof(float));
	assert(intensity_buffer);

	for (usize x = 0; x < width; x++) {
		for (usize y = 0; y < width; y++) {
			usize i = x / stride;
			usize j = y / stride;
			Arrow* tl = &arrow_buffer[((j + 0) * arrow_buffer_width) + (i + 0)];
			Arrow* tr = &arrow_buffer[((j + 0) * arrow_buffer_width) + (i + 1)];
			Arrow* bl = &arrow_buffer[((j + 1) * arrow_buffer_width) + (i + 0)];
			Arrow* br = &arrow_buffer[((j + 1) * arrow_buffer_width) + (i + 1)];
			float px = (float)(x % stride) / (float)stride;
			float py = (float)(y % stride) / (float)stride;
			float Ax = px - 0.0f; float Ay = py - 0.0f;
			float Bx = px - 1.0f; float By = py - 0.0f;
			float Cx = px - 0.0f; float Cy = py - 1.0f;
			float Dx = px - 1.0f; float Dy = py - 1.0f;
			float dot_a = (Ax * tl->dx) + (Ay * tl->dy);
			float dot_b = (Bx * tr->dx) + (By * tr->dy);
			float dot_c = (Cx * bl->dx) + (Cy * bl->dy);
			float dot_d = (Dx * br->dx) + (Dy * br->dy);
			float lerp_ab = lerp(dot_a, dot_b, smoothstep(px));
			float lerp_cd = lerp(dot_c, dot_d, smoothstep(px));
			float lerp_abcd = lerp(lerp_ab, lerp_cd, smoothstep(py));
			intensity_buffer[(y * width) + x] = lerp_abcd;
		}
	}

	float max = 0.0f;
	float min = FLT_MAX;
	for (usize x = 0; x < width; x++) {
		for (usize y = 0; y < width; y++) {
			float intensity = intensity_buffer[(y * width) + x];
			if (intensity > max) { max = intensity; }
			if (intensity < min) { min = intensity; }
		}
	}

	float difference = max - min;
	for (usize x = 0; x < width; x++) {
		for (usize y = 0; y < width; y++) {
			float* intensity = &intensity_buffer[(y * width) + x];
			*intensity = (*intensity - min) / difference;
		}
	}

	ROSE_Image* image = malloc(sizeof(ROSE_Image));
	assert(image);
	image->width = width;
	image->height = width;
	image->pixels = malloc(num_pixels * 4);
	assert(image->pixels);

	for (usize x = 0; x < width; x++) {
		for (usize y = 0; y < width; y++) {
			usize index = (y * width) + x;
			usize offset = index * 4;
			float intensity = intensity_buffer[index];
			image->pixels[offset + 0] = (u8)(intensity * 255.0f);
			image->pixels[offset + 1] = (u8)(intensity * 255.0f);
			image->pixels[offset + 2] = (u8)(intensity * 255.0f);
			image->pixels[offset + 3] = 255;
		}
	}
	
	free(arrow_buffer);
	free(intensity_buffer);
	return image;
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

void ROSE_ImageDestroy(ROSE_Image* image) {
	if (!image) { return; }
	free(image->pixels);
	free(image);
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

