
/*
 *   Source File [rose_image.c]
 */

#define ROSE_EXTERN
#include "rose.h"

ROSE_Perlin ROSE_PerlinCreate(uint32_t width, uint32_t height)
{
	float* noise = NULL;
	if ((width != 0) && (height != 0)) {
		size_t size = (size_t)width * height * sizeof(float);
		noise = malloc(size);
		memset(noise, 0, size);
	}

	return (ROSE_Perlin) {
		.seed = rand(),
		.divisions = 1,
		.layers = 1,
		.attenuation = 0.5f,
		.division_growth = 2.0f,
		._width = width,
		._height = height,
		._noise = noise,
	};
}

void ROSE_PerlinDestroy(ROSE_Perlin* perlin)
{
	if (perlin) {
		if (perlin->_noise) free(perlin->_noise);
		*perlin = (ROSE_Perlin) { 0 };
	}
}

void ROSE_PerlinGenerate(ROSE_Perlin* perlin)
{
	{
		if (!perlin->_noise)
			return;

		float divs = perlin->divisions;
		float layers = perlin->layers;
		float growth = perlin->division_growth;

		bool valid_info = (divs > 0)
			&& (layers > 0)
			&& (growth > 0.0f);

		if (!valid_info)
			ROSE_PANIC("ROSE_ImageCreatePerlin() invalid configuration! [divs: %u] [layers: %u] [growth: %f]", divs, layers, growth);
	}

	srand(perlin->seed);
	uint32_t width = perlin->_width;
	uint32_t height = perlin->_height;

	float amplitude = 1.0f;
	float layer_divisions = (float)perlin->divisions;
	for (uint32_t layer = 0; layer < perlin->layers; layer++) {
		ROSE__PerlinLayer(perlin->_noise, width, height, layer_divisions, amplitude);
		amplitude *= perlin->attenuation;
		layer_divisions *= perlin->division_growth;
		if ((layer_divisions >= width) || (layer_divisions >= height)) break;
		if (amplitude < 0.001f) break;
	}
}

float ROSE_PerlinGetIntensity(ROSE_Perlin* perlin, uint32_t xpos, uint32_t ypos)
{
	#if defined (ROSE_DEBUG)
	if ((xpos >= perlin->_width) || (ypos >= perlin->_height))
		ROSE_PANIC("pixel out of bounds! [%u, %u] [%u x %u]", xpos, ypos, perlin->_width, perlin->_height);
	#endif
	size_t index = ((size_t)ypos * perlin->_width) + xpos;
	return perlin->_noise[index];
}

void ROSE_PerlinSetIntensity(ROSE_Perlin* perlin, uint32_t xpos, uint32_t ypos, float intensity)
{
	#if defined (ROSE_DEBUG)
	if ((xpos >= perlin->_width) || (ypos >= perlin->_height))
		ROSE_PANIC("pixel out of bounds! [%u, %u] [%u x %u]", xpos, ypos, perlin->_width, perlin->_height);
	#endif
	size_t index = ((size_t)ypos * perlin->_width) + xpos;
	perlin->_noise[index] = intensity;
}

void ROSE__PerlinLayer(float* intensities, uint32_t width, uint32_t height, float divisions, float amplitude)
{
	typedef struct {
		float dx, dy;
	} Vec2;

	const float tau = 6.2831855f;
	static Vec2 precomputed_dirs[100] = { 0 };
	static bool precomputed = false;
	const uint32_t num_precomputed = sizeof(precomputed_dirs) / sizeof(precomputed_dirs[0]);
	if (!precomputed) {
		precomputed = true;
		for (uint32_t i = 0; i < num_precomputed; i++) {
			float angle = ((float)i / (float)num_precomputed) * tau;
			precomputed_dirs[i] = (Vec2){ cos(angle), sin(angle) };
		}
	}

	float cell_size = (float)width / divisions;
	float inv_cell = 1.0f / cell_size;
	size_t arrow_columns = (size_t)SDL_ceilf(divisions) + 1;
	size_t arrow_rows = (size_t)SDL_ceilf((float)height / cell_size) + 1;
	Vec2* arrows = malloc(arrow_columns * arrow_rows * sizeof(Vec2));
	
	for (size_t i = 0; i < arrow_columns * arrow_rows; i++) {
		arrows[i] = precomputed_dirs[rand() % num_precomputed];
	}

	for (uint32_t y = 0; y < height; y++) {
		float gy = ((float)y + 0.5f) * inv_cell;
		size_t j = (size_t)gy;
		if (j > arrow_rows - 2) j = arrow_rows - 2;
		float ypos = gy - (float)j;
		float smooth_ypos = (3.0f * ypos * ypos) - (2.0f * ypos * ypos * ypos);

		const Vec2* top = &arrows[j * arrow_columns];
		const Vec2* bot = top + arrow_columns;
		float* row = &intensities[(size_t)y * width];

		uint32_t x = 0;
		for (size_t i = 0; (i < arrow_columns - 1) && (x < width); i++) {
			uint32_t x_end = width;
			if (i < arrow_columns - 2) {
				x_end = (uint32_t)SDL_ceilf(((float)(i + 1) * cell_size) - 0.5f);
				if (x_end > width) x_end = width;
			}

			Vec2 top_left = top[i];
			Vec2 top_right = top[i + 1];
			Vec2 bottom_left = bot[i];
			Vec2 bottom_right = bot[i + 1];
			
			float a0 = ypos * top_left.dy;
			float a1 = top_left.dx;
			float b0 = (ypos * top_right.dy) - top_right.dx;
			float b1 = top_right.dx;
			float c0 = (ypos - 1.0f) * bottom_left.dy;
			float c1 = bottom_left.dx;
			float d0 = ((ypos - 1.0f) * bottom_right.dy) - bottom_right.dx;
			float d1 = bottom_right.dx;

			float left1 = amplitude * (a1 + ((c1 - a1) * smooth_ypos));
			float left0 = amplitude * (a0 + ((c0 - a0) * smooth_ypos));
			float right1 = amplitude * (b1 + ((d1 - b1) * smooth_ypos));
			float right0 = amplitude * (b0 + ((d0 - b0) * smooth_ypos));

			float origin = (float)i;
			for (; x < x_end; x++) {
				float xpos = (((float)x + 0.5f) * inv_cell) - origin;
				float smooth_xpos = (3.0f * xpos * xpos) - (2.0f * xpos * xpos * xpos);
				float left = (left1 * xpos) + left0;
				float right = (right1 * xpos) + right0;
				row[x] += left + (smooth_xpos * (right - left));
			}
		}
	}

	free(arrows);
}

