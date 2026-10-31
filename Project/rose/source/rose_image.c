
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

ROSE_Image ROSE_ImageCreatePerlin(uint32_t width, uint32_t height, ROSE_PerlinInfo* info)
{
	assert(info->divisions > 0);
	assert(info->layers > 0);
	assert(info->division_growth > 0.0f);
	srand(info->seed);
	ROSE_Image image = ROSE_ImageCreate(width, height);

	size_t num_pixels = (size_t)width * height;
	if (num_pixels == 0) return image;
	float* intensities = calloc(num_pixels, sizeof(float));

	float amplitude = 1.0f;
	float layer_divisions = (float)info->divisions;
	for (uint32_t layer = 0; layer < info->layers; layer++) {
		ROSE__PerlinNoiseLayer(intensities, width, height, layer_divisions, amplitude);
		amplitude *= info->attenuation;
		layer_divisions *= info->division_growth;
		if ((layer_divisions >= width) || (layer_divisions >= height)) break;
		if (amplitude < 0.001f) break;
	}

	float min = FLT_MAX;
	float max = -FLT_MAX;
	for (size_t i = 0; i < num_pixels; i++) {
		if (intensities[i] < min) min = intensities[i];
		if (intensities[i] > max) max = intensities[i];
	}

	float difference = max - min;
	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			float intensity = intensities[((size_t)y * width) + x];
			float value = (difference > 0.0f) ? ((intensity - min) / difference) : 0.0f;
			uint8_t c = (uint8_t)((value * 255.0f) + 0.5f);
			ROSE_ImageSetPixelRGBA(&image, x, y, (ROSE_RGBA) { c, c, c, 255 });
		}
	}

	free(intensities);
	return image;
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

void ROSE__PerlinNoiseLayer(float* intensities, uint32_t width, uint32_t height, float divisions, float amplitude)
{
	typedef struct {
		float dx, dy;
	} Arrow;

	float cell_size = (float)width / divisions;
	size_t arrow_columns = (size_t)SDL_ceilf(divisions) + 1;
	size_t arrow_rows = (size_t)SDL_ceilf((float)height / cell_size) + 1;
	Arrow* arrows = malloc(arrow_columns * arrow_rows * sizeof(Arrow));

	for (size_t i = 0; i < arrow_columns * arrow_rows; i++) {
		static const float tau = 6.28318535f;
		float angle = ((float)rand() / (float)RAND_MAX) * tau;
		arrows[i] = (Arrow) { cos(angle), sin(angle) };
	}

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			float gx = ((float)x + 0.5f) / cell_size;
			float gy = ((float)y + 0.5f) / cell_size;
			size_t i = (size_t)gx;
			size_t j = (size_t)gy;
			if (i > arrow_columns - 2) i = arrow_columns - 2;
			if (j > arrow_rows - 2) j = arrow_rows - 2;

			Arrow* tl = &arrows[((j + 0) * arrow_columns) + (i + 0)];
			Arrow* tr = &arrows[((j + 0) * arrow_columns) + (i + 1)];
			Arrow* bl = &arrows[((j + 1) * arrow_columns) + (i + 0)];
			Arrow* br = &arrows[((j + 1) * arrow_columns) + (i + 1)];
			float px = gx - (float)i;
			float py = gy - (float)j;
			float dot_a = ((px - 0.0f) * tl->dx) + ((py - 0.0f) * tl->dy);
			float dot_b = ((px - 1.0f) * tr->dx) + ((py - 0.0f) * tr->dy);
			float dot_c = ((px - 0.0f) * bl->dx) + ((py - 1.0f) * bl->dy);
			float dot_d = ((px - 1.0f) * br->dx) + ((py - 1.0f) * br->dy);

			static const float pi = 3.1415927f;
			float smooth_px = 0.5f + (sin((px * pi) - (pi * 0.5f)) * 0.5f);
			float smooth_py = 0.5f + (sin((py * pi) - (pi * 0.5f)) * 0.5f);
			float lerp_ab = (dot_a * (1.0f - smooth_px)) + (dot_b * smooth_px);
			float lerp_cd = (dot_c * (1.0f - smooth_px)) + (dot_d * smooth_px);
			float lerp_abcd = (lerp_ab * (1.0f - smooth_py)) + (lerp_cd * smooth_py);
			intensities[((size_t)y * width) + x] += amplitude * lerp_abcd;
		}
	}

	free(arrows);
}

