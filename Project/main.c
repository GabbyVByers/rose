
/*
 *   Source File [main.c]
 */

#include "rose.h"

#define MAP_WIDTH 350
#define MAP_PADDING 50
#define GUI_SCALE 3
#define GUI_PADDING 20
#define SLIDER_WIDTH 48

#define BUTTON_SMALL_WIDTH (ROSE_ASCII_WIDTH * 3)
#define BUTTON_LARGE_WIDTH (ROSE_ASCII_WIDTH * 7)
#define GUI_HEIGHT ((ROSE_ASCII_HEIGHT + 4) * GUI_SCALE)
#define GUI_HORIZONTAL_POSITION(x) (GUI_PADDING + (((BUTTON_SMALL_WIDTH * GUI_SCALE) + GUI_PADDING) * x))
#define GUI_VERTICAL_POSITION(x) (GUI_PADDING + ((GUI_HEIGHT + GUI_PADDING) * x))
#define GUI_LARGE_HORIZONTAL_POSITION(x) (GUI_PADDING + (((BUTTON_LARGE_WIDTH * GUI_SCALE) + GUI_PADDING) * x))

#define STRING_SAVE_MAP_PATH "map.png"
#define STRING_SAVE_MAP      "SAVE AS PNG"
#define STRING_RANDOM_SEED   "RANDOM SEED"

#define STRING_SCALE     "SCALE:     %d"
#define STRING_LAYERS    "LAYERS:    %d"
#define STRING_DETAIL    "DETAIL:    %.2f"
#define STRING_SEA_LEVEL "SEA LEVEL: %.2f"
#define STRING_INC  "+"
#define STRING_DEC  "-"

#define STRING_RAW       "RAW"
#define STRING_CLIP      "CLIP"
#define STRING_LAND      "LAND"
#define STRING_DEBURRED  "DEBURR"
#define STRING_FLOOD     "FLOOD"
#define STRING_NO_LAKES  "LAKES"
#define STRING_PROVINCES "PROVS"

typedef enum DisplayStage {
	RAW_NOISE,
	CLIPPED,
	RAW_LAND,
	DEBURRED,
	FLOOD_FILL,
	REMOVE_LAKES,
	PROVINCES,
} DisplayStage;

typedef struct Map {
	uint32_t width;
	float sea_level;
	ROSE_Image image;
	ROSE_Perlin perlin;
	ROSE_Sprite sprite;
	DisplayStage stage;
} Map;

void MapUpdate(Map* map)
{
	ROSE_ImageDestroy(&map->image);
	ROSE_PerlinGenerate(&map->perlin);

	if (map->stage == RAW_NOISE) {
		map->image = ROSE_ImageCreateFromPerlin(&map->perlin);
		ROSE_SpriteUpdateTexture(&map->sprite, &map->image);
		ROSE_ImageDestroy(&map->image);
		return;
	}

	int32_t width = map->width;
	const ROSE_RGBA color_red   = (ROSE_RGBA) { 255, 0, 0, 255 };
	const ROSE_RGBA color_green = (ROSE_RGBA) { 0, 255, 0, 255 };
	const ROSE_RGBA color_blue  = (ROSE_RGBA) { 0, 0, 255, 255 };

	{ /* Clipping */
		int32_t radius = width / 2;
		for (int32_t y = 0; y < width; y++) {
			for (int32_t x = 0; x < width; x++) {
				int32_t dx = x - radius;
				int32_t dy = y - radius;
				int32_t dist2 = (dx * dx) + (dy * dy);
				int32_t radius2 = radius * radius;
				if (dist2 > (radius * radius)) {
					ROSE_PerlinSetIntensity(&map->perlin, x, y, 0.0f);
				} else {
					float intensity = ROSE_PerlinGetIntensity(&map->perlin, x, y);
					float s = 1.0f - ((float)dist2 / radius2);
					float smooth_strength = (((3.0f * s) + 2.0f - (s * s * s)) / 2.0f) - 1.0f;
					intensity *= smooth_strength;
					ROSE_PerlinSetIntensity(&map->perlin, x, y, intensity);
				}
			}
		}

		if (map->stage == CLIPPED) {
			map->image = ROSE_ImageCreateFromPerlin(&map->perlin);
			ROSE_SpriteUpdateTexture(&map->sprite, &map->image);
			ROSE_ImageDestroy(&map->image);
			return;
		}
	}

	{ /* Sea Level */
		for (int32_t y = 0; y < width; y++) {
			for (int32_t x = 0; x < width; x++) {
				float intensity = ROSE_PerlinGetIntensity(&map->perlin, x, y);
				if (intensity < map->sea_level) {
					ROSE_PerlinSetIntensity(&map->perlin, x, y, 0.0f);
				} else {
					ROSE_PerlinSetIntensity(&map->perlin, x, y, 1.0f);
				}
			}
		}

		map->image = ROSE_ImageCreateFromPerlin(&map->perlin);
		for (int32_t y = 0; y < width; y++) {
			for (int32_t x = 0; x < width; x++) {
				float intensity = ROSE_PerlinGetIntensity(&map->perlin, x, y);
				if (intensity >= 0.95f) {
					ROSE_ImageSetPixelRGBA(&map->image, x, y, color_green);
				} else {
					ROSE_ImageSetPixelRGBA(&map->image, x, y, color_blue);
				}
			}
		}

		if (map->stage == RAW_LAND) {
			ROSE_SpriteUpdateTexture(&map->sprite, &map->image);
			return;
		}
	}

	{ /* Deburring */
		for (int32_t y = 0; y < width; y++) {
			for (int32_t x = 0; x < width; x++) {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&map->image, x, y);
				bool water = (color.b == 255);
				if (water) continue;

				ROSE_RGBA top    = ROSE_ImageGetPixelRGBA(&map->image, x, y - 1);
				ROSE_RGBA bottom = ROSE_ImageGetPixelRGBA(&map->image, x, y + 1);
				ROSE_RGBA left   = ROSE_ImageGetPixelRGBA(&map->image, x - 1, y);
				ROSE_RGBA right  = ROSE_ImageGetPixelRGBA(&map->image, x + 1, y);
				ROSE_RGBA top_right    = ROSE_ImageGetPixelRGBA(&map->image, x + 1, y - 1);
				ROSE_RGBA top_left     = ROSE_ImageGetPixelRGBA(&map->image, x - 1, y - 1);
				ROSE_RGBA bottom_right = ROSE_ImageGetPixelRGBA(&map->image, x + 1, y + 1);
				ROSE_RGBA bottom_left  = ROSE_ImageGetPixelRGBA(&map->image, x - 1, y + 1);

				bool water_top    = (top.b == 255);
				bool water_bottom = (bottom.b == 255);
				bool water_left   = (left.b == 255);
				bool water_right  = (right.b == 255);
				bool land_top_right    = (top_right.g == 255);
				bool land_top_left     = (top_left.g == 255);
				bool land_bottom_right = (bottom_right.g == 255);
				bool land_bottom_left  = (bottom_left.g == 255);

				bool tiny_island = water_top
					&& water_bottom
					&& water_left
					&& water_right
					&& !land_top_right
					&& !land_top_left
					&& !land_bottom_right
					&& !land_bottom_left;

				if (tiny_island) {
					ROSE_ImageSetPixelRGBA(&map->image, x, y, color_blue);
					continue;
				}

				if (water_top && water_right) {
					if (land_top_right) {
						ROSE_ImageSetPixelRGBA(&map->image, x, y, color_blue);
						continue;
					}
				}

				if (water_top && water_left) {
					if (land_top_left) {
						ROSE_ImageSetPixelRGBA(&map->image, x, y, color_blue);
						continue;
					}
				}

				if (water_bottom && water_right) {
					if (land_bottom_right) {
						ROSE_ImageSetPixelRGBA(&map->image, x, y, color_blue);
						continue;
					}
				}

				if (water_bottom && water_left) {
					if (land_bottom_left) {
						ROSE_ImageSetPixelRGBA(&map->image, x, y, color_blue);
						continue;
					}
				}
			}
		}

		if (map->stage == DEBURRED) {
			ROSE_SpriteUpdateTexture(&map->sprite, &map->image);
			return;
		}
	}

	{ /* Flood Fill */
		typedef struct {
			uint32_t x;
			uint32_t y;
		} Vec2;

		const int32_t neighbour_offsets[4][2] = {
			{  0, -1 },
			{  0,  1 },
			{ -1,  0 },
			{  1,  0 }
		};

		uint32_t position_queue_head = 0;
		uint32_t position_queue_size = 0;
		uint32_t position_queue_capacity = 1024;
		Vec2* position_queue = malloc(position_queue_capacity * sizeof(Vec2));

		Vec2 start = (Vec2) { 0, 0 };
		ROSE_ImageSetPixelRGBA(&map->image, start.x, start.y, color_red);
		position_queue[position_queue_size++] = start;

		while (position_queue_head < position_queue_size) {
			Vec2 position = position_queue[position_queue_head++];

			for (int32_t i = 0; i < 4; i++) {
				uint32_t next_xpos = position.x + (uint32_t)neighbour_offsets[i][0];
				uint32_t next_ypos = position.y + (uint32_t)neighbour_offsets[i][1];
				if ((next_xpos >= width) || (next_ypos >= width))
					continue;

				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&map->image, next_xpos, next_ypos);
				bool is_water = (color.b == 255);
				if (!is_water) continue;

				if (position_queue_size == position_queue_capacity) {
					position_queue_capacity *= 2;
					Vec2* new_queue = realloc(position_queue, position_queue_capacity * sizeof(Vec2));
					position_queue = new_queue;
				}

				ROSE_ImageSetPixelRGBA(&map->image, next_xpos, next_ypos, color_red);
				position_queue[position_queue_size++] = (Vec2){ next_xpos, next_ypos };
			}
		}

		free(position_queue);
		if (map->stage == FLOOD_FILL) {
			ROSE_SpriteUpdateTexture(&map->sprite, &map->image);
			return;
		}
	}

	{ /* Remove Lakes */
		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&map->image, x, y);
				bool is_lake = (color.b == 255);
				if (is_lake) {
					ROSE_ImageSetPixelRGBA(&map->image, x, y, color_green);
				}
			}
		}

		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&map->image, x, y);
				bool is_blood = (color.r == 255);
				if (is_blood) {
					ROSE_ImageSetPixelRGBA(&map->image, x, y, color_blue);
				}
			}
		}

		if (map->stage == REMOVE_LAKES) {
			ROSE_SpriteUpdateTexture(&map->sprite, &map->image);
			return;
		}
	}

	{ /* Province Flood */
		typedef struct {
			uint32_t x;
			uint32_t y;
		} Vec2;

		typedef struct {
			ROSE_RGBA color;
			Vec2* frontier;
			uint32_t pixel_count;
			uint32_t frontier_size;
			uint32_t frontier_capacity;
		} PixelProv;

		const uint32_t province_spacing = 8;
		const uint32_t seeds_per_row = (width + province_spacing - 1) / province_spacing;
		
		uint32_t province_count = 0;
		uint32_t province_capacity = seeds_per_row * seeds_per_row;
		PixelProv* provinces = malloc(province_capacity * sizeof(PixelProv));

		for (uint32_t y = 0; y < width; y += province_spacing) {
			for (uint32_t x = 0; x < width; x += province_spacing) {
				if ((y / province_spacing) % 2 == 0)
					x += (province_spacing / 2);

				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&map->image, x, y);
				bool is_land = (color.g == 255);
				if (!is_land) continue;

				uint8_t r = (uint8_t)(55 + (rand() % 200));
				uint8_t g = (uint8_t)(55 + (rand() % 200));
				uint8_t b = (uint8_t)(55 + (rand() % 200));

				PixelProv province = (PixelProv) {
					.color = (ROSE_RGBA) { r, g, b, 255 },
					.pixel_count = 1,
					.frontier_size = 0,
					.frontier_capacity = 32,
				};

				ROSE_ImageSetPixelRGBA(&map->image, x, y, province.color);
				province.frontier = malloc(province.frontier_capacity * sizeof(Vec2));
				province.frontier[province.frontier_size++] = (Vec2) { x, y };
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
					Vec2 position = province->frontier[index];

					uint32_t open_directions[4];
					uint32_t open_count = 0;
					for (uint32_t i = 0; i < 4; i++) {
						uint32_t next_xpos = position.x + (uint32_t)neighbour_offsets[i][0];
						uint32_t next_ypos = position.y + (uint32_t)neighbour_offsets[i][1];
						if ((next_xpos >= width) || (next_ypos >= width))
							continue;

						ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&map->image, next_xpos, next_ypos);
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
						Vec2* new_frontier = realloc(province->frontier, province->frontier_capacity * sizeof(Vec2));
						province->frontier = new_frontier;
					}

					ROSE_ImageSetPixelRGBA(&map->image, next_xpos, next_ypos, province->color);
					province->pixel_count++;
					province->frontier[province->frontier_size++] = (Vec2) { next_xpos, next_ypos };
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
						ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&map->image, x, y);
						ROSE_RGBA pcolor = province->color;
						bool is_province = (color.r == pcolor.r) && (color.g == pcolor.g) && (color.b == pcolor.b);
						if (!is_province) continue;
						ROSE_ImageSetPixelRGBA(&map->image, x, y, color_blue);
					}
				}
			}
		}

		for (uint32_t p = 0; p < province_count; p++) {
			free(provinces[p].frontier);
		} free(provinces);

		for (uint32_t y = 0; y < width; y++) {
			for (uint32_t x = 0; x < width; x++) {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&map->image, x, y);
				bool is_land = (color.g == 255);
				if (!is_land) continue;
				ROSE_ImageSetPixelRGBA(&map->image, x, y, color_blue);
			}
		}

		if (map->stage == PROVINCES) {
			ROSE_SpriteUpdateTexture(&map->sprite, &map->image);
			return;
		}
	}

	ROSE_SpriteUpdateTexture(&map->sprite, &map->image);
}

int main(void)
{
	ROSE_Init("Pearl", 1600, 900);
	ROSE_WindowVSync(false);

	Map map = (Map) {
		.width = MAP_WIDTH,
		.sea_level = 0.5f,
		.perlin = ROSE_PerlinCreate(MAP_WIDTH, MAP_WIDTH),
		.sprite = ROSE_SpriteCreateEmpty(MAP_WIDTH, MAP_WIDTH),
	};

	map.sprite.scale = 2;
	map.perlin.divisions = 5;
	map.perlin.layers = 5;
	map.perlin.attenuation = 0.8f;
	MapUpdate(&map);

	// [  SAVE MAP   ]
	// [ RANDOM SEED ]
	ROSE_Button button_save_map      = ROSE_ButtonCreate(STRING_SAVE_MAP);
	ROSE_Button button_random_seed   = ROSE_ButtonCreate(STRING_RANDOM_SEED);
	button_save_map.scale            = GUI_SCALE;
	button_random_seed.scale         = GUI_SCALE;
	button_save_map.xpos             = GUI_HORIZONTAL_POSITION(0);
	button_random_seed.xpos          = GUI_HORIZONTAL_POSITION(0);
	button_save_map.ypos             = GUI_VERTICAL_POSITION(0);
	button_random_seed.ypos          = GUI_VERTICAL_POSITION(1);
	
	// [-] [+] SCALE: #
	ROSE_Button button_init_divs_dec = ROSE_ButtonCreate(STRING_DEC);
	ROSE_Button button_init_divs_inc = ROSE_ButtonCreate(STRING_INC);
	ROSE_Text   text_init_divs       = ROSE_TextCreate(STRING_SCALE, map.perlin.divisions);
	button_init_divs_dec.scale       = GUI_SCALE;
	button_init_divs_inc.scale       = GUI_SCALE;
	text_init_divs.scale             = GUI_SCALE;
	button_init_divs_dec.xpos        = GUI_HORIZONTAL_POSITION(0);
	button_init_divs_inc.xpos        = GUI_HORIZONTAL_POSITION(1);
	text_init_divs.xpos              = GUI_HORIZONTAL_POSITION(2);
	button_init_divs_dec.ypos        = GUI_VERTICAL_POSITION(2);
	button_init_divs_inc.ypos        = GUI_VERTICAL_POSITION(2);
	text_init_divs.ypos              = GUI_VERTICAL_POSITION(2) + (2 * GUI_SCALE);
	button_init_divs_dec.width       = BUTTON_SMALL_WIDTH;
	button_init_divs_inc.width       = BUTTON_SMALL_WIDTH;
	
	// [-] [+] LAYERS: #
	ROSE_Button button_layers_dec    = ROSE_ButtonCreate(STRING_DEC);
	ROSE_Button button_layers_inc    = ROSE_ButtonCreate(STRING_INC);
	ROSE_Text   text_layers          = ROSE_TextCreate(STRING_LAYERS, map.perlin.layers);
	button_layers_dec.scale          = GUI_SCALE;
	button_layers_inc.scale          = GUI_SCALE;
	text_layers.scale                = GUI_SCALE;
	button_layers_dec.xpos           = GUI_HORIZONTAL_POSITION(0);
	button_layers_inc.xpos           = GUI_HORIZONTAL_POSITION(1);
	text_layers.xpos                 = GUI_HORIZONTAL_POSITION(2);
	button_layers_dec.ypos           = GUI_VERTICAL_POSITION(3);
	button_layers_inc.ypos           = GUI_VERTICAL_POSITION(3);
	text_layers.ypos                 = GUI_VERTICAL_POSITION(3) + (2 * GUI_SCALE);
	button_layers_dec.width          = BUTTON_SMALL_WIDTH;
	button_layers_inc.width          = BUTTON_SMALL_WIDTH;
	
	// --[]--- DETAIL: #
	ROSE_Slider slider_attenuation   = ROSE_SliderCreate();
	ROSE_Text   text_attenuation     = ROSE_TextCreate(STRING_DETAIL, map.perlin.attenuation);
	slider_attenuation.subject       = &map.perlin.attenuation;
	slider_attenuation.scale         = GUI_SCALE;
	text_attenuation.scale           = GUI_SCALE;
	slider_attenuation.xpos          = GUI_HORIZONTAL_POSITION(0);
	text_attenuation.xpos            = GUI_HORIZONTAL_POSITION(2);
	slider_attenuation.ypos          = GUI_VERTICAL_POSITION(4);
	text_attenuation.ypos            = GUI_VERTICAL_POSITION(4) + (2 * GUI_SCALE);
	slider_attenuation.width         = SLIDER_WIDTH;
	
	// --[]--- SEA LEVEL: #
	ROSE_Slider slider_sea_level     = ROSE_SliderCreate();
	ROSE_Text   text_sea_level       = ROSE_TextCreate(STRING_SEA_LEVEL, map.sea_level);
	slider_sea_level.subject         = &map.sea_level;
	slider_sea_level.scale           = GUI_SCALE;
	text_sea_level.scale             = GUI_SCALE;
	slider_sea_level.xpos            = GUI_HORIZONTAL_POSITION(0);
	text_sea_level.xpos              = GUI_HORIZONTAL_POSITION(2);
	slider_sea_level.ypos            = GUI_VERTICAL_POSITION(5);
	text_sea_level.ypos              = GUI_VERTICAL_POSITION(5) + (2 * GUI_SCALE);
	slider_sea_level.width           = SLIDER_WIDTH;
	
	// [ RAW ] [ CLIP ] [ LAND ] [ DEBURR ]
	ROSE_Button button_show_raw       = ROSE_ButtonCreate(STRING_RAW);
	ROSE_Button button_show_clipped   = ROSE_ButtonCreate(STRING_CLIP);
	ROSE_Button button_show_land      = ROSE_ButtonCreate(STRING_LAND);
	ROSE_Button button_show_deburred  = ROSE_ButtonCreate(STRING_DEBURRED);
	ROSE_Button button_show_flood     = ROSE_ButtonCreate(STRING_FLOOD);
	ROSE_Button button_show_no_lakes  = ROSE_ButtonCreate(STRING_NO_LAKES);
	ROSE_Button button_show_provinces = ROSE_ButtonCreate(STRING_PROVINCES);
	button_show_raw.scale            = GUI_SCALE;
	button_show_clipped.scale        = GUI_SCALE;
	button_show_land.scale           = GUI_SCALE;
	button_show_deburred.scale       = GUI_SCALE;
	button_show_flood.scale          = GUI_SCALE;
	button_show_no_lakes.scale       = GUI_SCALE;
	button_show_provinces.scale      = GUI_SCALE;
	button_show_raw.width            = BUTTON_LARGE_WIDTH;
	button_show_clipped.width        = BUTTON_LARGE_WIDTH;
	button_show_land.width           = BUTTON_LARGE_WIDTH;
	button_show_deburred.width       = BUTTON_LARGE_WIDTH;
	button_show_flood.width          = BUTTON_LARGE_WIDTH;
	button_show_no_lakes.width       = BUTTON_LARGE_WIDTH;
	button_show_provinces.width      = BUTTON_LARGE_WIDTH;

	while (ROSE_WindowPollEvents())
	{
		const bool pressed_button_save_map       = ROSE_ButtonPressed(&button_save_map);
		const bool pressed_button_random_seed    = ROSE_ButtonPressed(&button_random_seed);
		const bool pressed_button_init_divs_dec  = ROSE_ButtonPressed(&button_init_divs_dec);
		const bool pressed_button_init_divs_inc  = ROSE_ButtonPressed(&button_init_divs_inc);
		const bool pressed_button_layers_dec     = ROSE_ButtonPressed(&button_layers_dec);
		const bool pressed_button_layers_inc     = ROSE_ButtonPressed(&button_layers_inc);
		const bool pressed_button_show_raw       = ROSE_ButtonPressed(&button_show_raw);
		const bool pressed_button_show_clipped   = ROSE_ButtonPressed(&button_show_clipped);
		const bool pressed_button_show_land      = ROSE_ButtonPressed(&button_show_land);
		const bool pressed_button_show_deburred  = ROSE_ButtonPressed(&button_show_deburred);
		const bool pressed_button_show_flood     = ROSE_ButtonPressed(&button_show_flood);
		const bool pressed_button_show_no_lakes  = ROSE_ButtonPressed(&button_show_no_lakes);
		const bool pressed_button_show_provinces = ROSE_ButtonPressed(&button_show_provinces);

		if (pressed_button_save_map) {
			if (&map.image._pixels) {
				ROSE_ImageSavePNG(&map.image, STRING_SAVE_MAP_PATH);
			}
		}

		if (pressed_button_random_seed) {
			map.perlin.seed = rand();
		}

		if (pressed_button_init_divs_dec) {
			if (map.perlin.divisions > 1) {
				map.perlin.divisions--;
				ROSE_TextUpdateString(&text_init_divs, STRING_SCALE, map.perlin.divisions);
			}
		}

		if (pressed_button_init_divs_inc) {
			if (map.perlin.divisions < 20) {
				map.perlin.divisions++;
				ROSE_TextUpdateString(&text_init_divs, STRING_SCALE, map.perlin.divisions);
			}
		}

		if (pressed_button_layers_dec) {
			if (map.perlin.layers > 1) {
				map.perlin.layers--;
				ROSE_TextUpdateString(&text_layers, STRING_LAYERS, map.perlin.layers);
			}
		}

		if (pressed_button_layers_inc) {
			if (map.perlin.layers < 8) {
				map.perlin.layers++;
				ROSE_TextUpdateString(&text_layers, STRING_LAYERS, map.perlin.layers);
			}
		}

		if (pressed_button_show_raw)       { map.stage = RAW_NOISE;    }
		if (pressed_button_show_clipped)   { map.stage = CLIPPED;      }
		if (pressed_button_show_land)      { map.stage = RAW_LAND;     }
		if (pressed_button_show_deburred)  { map.stage = DEBURRED;     }
		if (pressed_button_show_flood)     { map.stage = FLOOD_FILL;   }
		if (pressed_button_show_no_lakes)  { map.stage = REMOVE_LAKES; }
		if (pressed_button_show_provinces) { map.stage = PROVINCES;    }

		MapUpdate(&map);
		ROSE_WindowBeginFrame(ROSE_COLOR_BLACK);
		{
			map.sprite.xpos = rose_screen.width - (MAP_PADDING + (map.width * map.sprite.scale));
			map.sprite.ypos = MAP_PADDING;

			button_show_raw.xpos       = GUI_LARGE_HORIZONTAL_POSITION(0) + map.sprite.xpos;
			button_show_clipped.xpos   = GUI_LARGE_HORIZONTAL_POSITION(1) + map.sprite.xpos;
			button_show_land.xpos      = GUI_LARGE_HORIZONTAL_POSITION(2) + map.sprite.xpos;
			button_show_deburred.xpos  = GUI_LARGE_HORIZONTAL_POSITION(3) + map.sprite.xpos;
			button_show_flood.xpos     = GUI_LARGE_HORIZONTAL_POSITION(0) + map.sprite.xpos;
			button_show_no_lakes.xpos  = GUI_LARGE_HORIZONTAL_POSITION(1) + map.sprite.xpos;
			button_show_provinces.xpos = GUI_LARGE_HORIZONTAL_POSITION(2) + map.sprite.xpos;

			button_show_raw.ypos       = GUI_VERTICAL_POSITION(0) + map.sprite.ypos + (map.width * map.sprite.scale);
			button_show_clipped.ypos   = GUI_VERTICAL_POSITION(0) + map.sprite.ypos + (map.width * map.sprite.scale);
			button_show_land.ypos      = GUI_VERTICAL_POSITION(0) + map.sprite.ypos + (map.width * map.sprite.scale);
			button_show_deburred.ypos  = GUI_VERTICAL_POSITION(0) + map.sprite.ypos + (map.width * map.sprite.scale);
			button_show_flood.ypos     = GUI_VERTICAL_POSITION(1) + map.sprite.ypos + (map.width * map.sprite.scale);
			button_show_no_lakes.ypos  = GUI_VERTICAL_POSITION(1) + map.sprite.ypos + (map.width * map.sprite.scale);
			button_show_provinces.ypos = GUI_VERTICAL_POSITION(1) + map.sprite.ypos + (map.width * map.sprite.scale);

			ROSE_ButtonDraw(&button_save_map);
			ROSE_ButtonDraw(&button_random_seed);

			ROSE_ButtonDraw(&button_init_divs_dec);
			ROSE_ButtonDraw(&button_init_divs_inc);
			ROSE_TextDraw(&text_init_divs);

			ROSE_ButtonDraw(&button_layers_dec);
			ROSE_ButtonDraw(&button_layers_inc);
			ROSE_TextDraw(&text_layers);

			ROSE_SliderDraw(&slider_attenuation);
			ROSE_TextUpdateString(&text_attenuation, STRING_DETAIL, map.perlin.attenuation);
			ROSE_TextDraw(&text_attenuation);

			ROSE_SliderDraw(&slider_sea_level);
			ROSE_TextUpdateString(&text_sea_level, STRING_SEA_LEVEL, map.sea_level);
			ROSE_TextDraw(&text_sea_level);

			ROSE_SpriteDraw(&map.sprite);
			ROSE_ButtonDraw(&button_show_raw);
			ROSE_ButtonDraw(&button_show_clipped);
			ROSE_ButtonDraw(&button_show_land);
			ROSE_ButtonDraw(&button_show_deburred);
			ROSE_ButtonDraw(&button_show_flood);
			ROSE_ButtonDraw(&button_show_no_lakes);
			ROSE_ButtonDraw(&button_show_provinces);
		}
		ROSE_WindowEndFrame();
	}

	ROSE_Quit();
	return EXIT_SUCCESS;
}

