
/*
 *   Source File [rose_perlin.c]
 */

#include "rose.h"

static float lerp(float left, float right, float weight) {
	return (left * (1.0f - weight)) + (right * weight);
}

static float smoothstep(float value) {
	static const float pi = 3.1415927f;
	return 0.5f + (sin((value * pi) - (pi * 0.5f)) * 0.5f);
}

ROSE_Image* ROSE_ImagePerlinNoise(i32 _width, i32 _divisions) {
	typedef struct {
		float px, py;
		float dx, dy;
	} Arrow;

	usize width = (usize)_width;
	usize divisions = (usize)_divisions;
	usize arrow_buffer_width = divisions + 3;
	usize num_arrows = arrow_buffer_width * arrow_buffer_width;
	Arrow* arrow_buffer = malloc(num_arrows * sizeof(Arrow));

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

	usize stride = (width / divisions) + 1;
	usize num_pixels = width * width;
	float* intensity_buffer = malloc(num_pixels * sizeof(float));

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

	float max = -FLT_MAX;
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
	image->width = width;
	image->height = width;
	image->pixels = malloc(num_pixels * 4);

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

