
/*
 *   Source File [map_generator.c]
 */

#include "map.h"

typedef struct Vec2u {
	uint32_t x;
	uint32_t y;
} Vec2u;

static ROSE_RGBA getpixelsafe(ROSE_Image* image, int32_t x, int32_t y)
{
	bool bad_bounds =
		(x < 0) ||
		(y < 0) ||
		((uint32_t)x >= image->_width) ||
		((uint32_t)y >= image->_height);

	if (bad_bounds)
		return (ROSE_RGBA) { 0, 0, 255, 255 };

	return ROSE_ImageGetPixelRGBA(image, (uint32_t)x, (uint32_t)y);
}

ROSE_Image MAP_GenerateImage(MapInfo* map_info)
{
	const uint32_t width     = map_info->map_width;
	const float    sea_level = map_info->map_sea_level;

	const uint32_t seed             = map_info->perlin_seed;
	const uint32_t divisions        = map_info->perlin_divisions;
	const uint32_t layers           = map_info->perlin_layers;
	const float    attenuation      = map_info->perlin_attenuation;
	const uint32_t province_spacing = map_info->province_spacing;
	const uint32_t province_drift   = map_info->province_drift;

	const ROSE_RGBA color_red   = (ROSE_RGBA) { 255, 0, 0, 255 };
	const ROSE_RGBA color_green = (ROSE_RGBA) { 0, 255, 0, 255 };
	const ROSE_RGBA color_blue  = (ROSE_RGBA) { 0, 0, 255, 255 };

	ROSE_Perlin perlin = ROSE_PerlinCreate(width, width);
	ROSE_Image  image = (ROSE_Image) { 0 };

	perlin.seed        = seed;
	perlin.divisions   = divisions;
	perlin.layers      = layers;
	perlin.attenuation = attenuation;
	ROSE_PerlinGenerate(&perlin);

	{
		const int32_t radius = ((int32_t)width / 2);
		for (int32_t y = 0; y < width; y++) {
			for (int32_t x = 0; x < width; x++) {
				int32_t dx = x - radius;
				int32_t dy = y - radius;
				int32_t dist2 = (dx * dx) + (dy * dy);
				int32_t radius2 = radius * radius;

				if (dist2 > radius2) {
					ROSE_PerlinSetIntensity(&perlin, x, y, 0.0f);
				} else {
					float intensity = ROSE_PerlinGetIntensity(&perlin, x, y);
					float s = 1.0f - ((float)dist2 / radius2);
					float smooth_strength = (((3.0f * s) + 2.0f - (s * s * s)) / 2.0f) - 1.0f;
					intensity *= smooth_strength;
					ROSE_PerlinSetIntensity(&perlin, x, y, intensity);
				}
			}
		}
	}

	{
		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				float intensity = ROSE_PerlinGetIntensity(&perlin, x, y);
				if (intensity < sea_level) {
					ROSE_PerlinSetIntensity(&perlin, x, y, 0.0f);
				} else {
					ROSE_PerlinSetIntensity(&perlin, x, y, 1.0f);
				}
			}
		}

		image = ROSE_ImageCreateFromPerlin(&perlin);

		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				float intensity = ROSE_PerlinGetIntensity(&perlin, x, y);
				if (intensity >= 0.95f) {
					ROSE_ImageSetPixelRGBA(&image, x, y, color_green);
				} else {
					ROSE_ImageSetPixelRGBA(&image, x, y, color_blue);
				}
			}
		}

		ROSE_PerlinDestroy(&perlin);
	}

	{
		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, x, y);
				bool water = (color.b == 255);
				if (water) continue;

				ROSE_RGBA top    = ROSE_ImageGetPixelRGBA(&image, x, y - 1);
				ROSE_RGBA bottom = ROSE_ImageGetPixelRGBA(&image, x, y + 1);
				ROSE_RGBA left   = ROSE_ImageGetPixelRGBA(&image, x - 1, y);
				ROSE_RGBA right  = ROSE_ImageGetPixelRGBA(&image, x + 1, y);
				ROSE_RGBA top_right    = ROSE_ImageGetPixelRGBA(&image, x + 1, y - 1);
				ROSE_RGBA top_left     = ROSE_ImageGetPixelRGBA(&image, x - 1, y - 1);
				ROSE_RGBA bottom_right = ROSE_ImageGetPixelRGBA(&image, x + 1, y + 1);
				ROSE_RGBA bottom_left  = ROSE_ImageGetPixelRGBA(&image, x - 1, y + 1);

				bool water_top    = (top.b    == 255);
				bool water_bottom = (bottom.b == 255);
				bool water_left   = (left.b   == 255);
				bool water_right  = (right.b  == 255);
				bool land_top_right    = (top_right.g    == 255);
				bool land_top_left     = (top_left.g     == 255);
				bool land_bottom_right = (bottom_right.g == 255);
				bool land_bottom_left  = (bottom_left.g  == 255);

				bool tiny_island = water_top
					&& water_bottom
					&& water_left
					&& water_right
					&& !land_top_right
					&& !land_top_left
					&& !land_bottom_right
					&& !land_bottom_left;

				bool deburr = tiny_island
					|| (water_top    && water_right && land_top_right   )
					|| (water_top    && water_left  && land_top_left    )
					|| (water_bottom && water_right && land_bottom_right)
					|| (water_bottom && water_left  && land_bottom_left );

				if (deburr) {
					ROSE_ImageSetPixelRGBA(&image, x, y, color_blue);
					continue;
				}
			}
		}
	}

	{
		const int32_t neighbour_offsets[4][2] = {
			{  0, -1 },
			{  0,  1 },
			{ -1,  0 },
			{  1,  0 }
		};

		uint32_t position_queue_head = 0;
		uint32_t position_queue_size = 0;
		uint32_t position_queue_capacity = 1024;
		Vec2u* position_queue = malloc(position_queue_capacity * sizeof(Vec2u));

		ROSE_ImageSetPixelRGBA(&image, 0, 0, color_red);
		position_queue[position_queue_size++] = (Vec2u) { 0, 0 };

		while (position_queue_head < position_queue_size) {
			Vec2u position = position_queue[position_queue_head++];

			for (int32_t i = 0; i < 4; i++) {
				uint32_t next_xpos = position.x + (uint32_t)neighbour_offsets[i][0];
				uint32_t next_ypos = position.y + (uint32_t)neighbour_offsets[i][1];
				if ((next_xpos >= width) || (next_ypos >= width)) continue;

				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, next_xpos, next_ypos);
				bool is_water = (color.b == 255);
				if (!is_water) continue;

				if (position_queue_size == position_queue_capacity) {
					position_queue_capacity *= 2;
					Vec2u* new_queue = realloc(position_queue, position_queue_capacity * sizeof(Vec2u));
					position_queue = new_queue;
				}

				ROSE_ImageSetPixelRGBA(&image, next_xpos, next_ypos, color_red);
				position_queue[position_queue_size++] = (Vec2u){ next_xpos, next_ypos };
			}
		}

		free(position_queue);
	}

	{
		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, x, y);
				bool is_lake = (color.b == 255);
				if (is_lake) {
					ROSE_ImageSetPixelRGBA(&image, x, y, color_green);
				}
			}
		}

		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, x, y);
				bool is_blood = (color.r == 255);
				if (is_blood) {
					ROSE_ImageSetPixelRGBA(&image, x, y, color_blue);
				}
			}
		}
	}

	{
		typedef struct {
			ROSE_RGBA color;
			Vec2u*     frontier;
			uint32_t  pixel_count;
			uint32_t  frontier_size;
			uint32_t  frontier_capacity;
		} PixelProv;

		const uint32_t seeds_per_row =
			(width + province_spacing - 1)
			/ province_spacing;

		uint32_t province_count = 0;
		uint32_t province_capacity = seeds_per_row * seeds_per_row;
		PixelProv* provinces = malloc(province_capacity * sizeof(PixelProv));

		for (int32_t y = 0; y < width; y += province_spacing) {
			
			const bool even_row = (((y / province_spacing) % 2) == 0);
			const int32_t x_start = (even_row) ? (province_spacing / 2) : 0;
			
			for (int32_t x = x_start; x < width; x += province_spacing) {
				float dx = (2.0f * (float)province_spacing * ((float)rand() / (float)RAND_MAX)) - 1.0f;
				float dy = (2.0f * (float)province_spacing * ((float)rand() / (float)RAND_MAX)) - 1.0f;
				ROSE_RGBA color = getpixelsafe(&image, (int32_t)(x + dx), (int32_t)(y + dy));

				bool is_land = (color.g == 255);
				if (!is_land) continue;

				uint8_t r = 0;
				uint8_t g = 0;
				uint8_t b = 0;

				for (;;) {
					r = (uint8_t)(55 + (rand() % 200));
					g = (uint8_t)(55 + (rand() % 200));
					b = (uint8_t)(55 + (rand() % 200));
					bool color_available = true;

					for (uint32_t i = 0; i < province_count; i++) {
						PixelProv* province = &provinces[i];
						ROSE_RGBA curr = province->color;
						if ((r == curr.r) && (g == curr.g) && (b == curr.b)) {
							color_available = false;
							break;
						}
					}

					if (color_available)
						break;
				}

				PixelProv province = (PixelProv){
					.color = (ROSE_RGBA) { r, g, b, 255 },
					.pixel_count = 1,
					.frontier_size = 0,
					.frontier_capacity = 32,
				};

				ROSE_ImageSetPixelRGBA(&image, x, y, province.color);
				province.frontier = malloc(province.frontier_capacity * sizeof(Vec2u));
				province.frontier[province.frontier_size++] = (Vec2u){ x, y };
				provinces[province_count++] = province;
			}
		}

		const int32_t neighbour_offsets[4][2] = {
			{  0, -1 },
			{  0,  1 },
			{ -1,  0 },
			{  1,  0 }
		};

		uint32_t active_count = province_count;
		while (active_count > 0) {
			active_count = 0;

			for (uint32_t p = 0; p < province_count; p++) {
				PixelProv* province = &provinces[p];

				while (province->frontier_size > 0) {
					uint32_t index = rand() % province->frontier_size;
					Vec2u position = province->frontier[index];

					uint32_t open_directions[4];
					uint32_t open_count = 0;
					for (uint32_t i = 0; i < 4; i++) {
						uint32_t next_xpos = position.x + (uint32_t)neighbour_offsets[i][0];
						uint32_t next_ypos = position.y + (uint32_t)neighbour_offsets[i][1];
						if ((next_xpos >= width) || (next_ypos >= width))
							continue;

						ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, next_xpos, next_ypos);
						bool is_land = (color.g == 255);
						if (is_land) open_directions[open_count++] = i;
					}

					if (open_count == 0) {
						province->frontier[index] = province->frontier[--province->frontier_size];
						continue;
					}

					uint32_t direction = open_directions[(uint32_t)rand() % open_count];
					uint32_t next_xpos = position.x + (uint32_t)neighbour_offsets[direction][0];
					uint32_t next_ypos = position.y + (uint32_t)neighbour_offsets[direction][1];

					if (province->frontier_size == province->frontier_capacity) {
						province->frontier_capacity *= 2;
						Vec2u* new_frontier = realloc(province->frontier, province->frontier_capacity * sizeof(Vec2u));
						province->frontier = new_frontier;
					}

					ROSE_ImageSetPixelRGBA(&image, next_xpos, next_ypos, province->color);
					province->pixel_count++;
					province->frontier[province->frontier_size++] = (Vec2u){ next_xpos, next_ypos };
					break;
				}

				if (province->frontier_size > 0)
					active_count++;
			}
		}

		uint32_t pixel_sum = 0;
		for (uint32_t p = 0; p < province_count; p++) {
			pixel_sum += provinces[p].pixel_count;
		}

		uint32_t mean_pixels = pixel_sum / province_count;
		for (uint32_t p = 0; p < province_count; p++) {
			PixelProv* province = &provinces[p];
			if (province->pixel_count < (mean_pixels / 2)) {
				for (uint32_t y = 0; y < width; y++) {
					for (uint32_t x = 0; x < width; x++) {
						ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, x, y);
						ROSE_RGBA pcolor = province->color;
						bool is_province = (color.r == pcolor.r) && (color.g == pcolor.g) && (color.b == pcolor.b);
						if (!is_province) continue;
						ROSE_ImageSetPixelRGBA(&image, x, y, color_blue);
					}
				}
			}
		}

		for (uint32_t p = 0; p < province_count; p++) {
			free(provinces[p].frontier);
		} free(provinces);

		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&image, x, y);
				bool is_land = (color.g == 255);
				if (!is_land) continue;
				ROSE_ImageSetPixelRGBA(&image, x, y, color_blue);
			}
		}
	}

	return image;
}

