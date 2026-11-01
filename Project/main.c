
/*
 *   Source File [main.c]
 */

#include "rose.h"

#define MAP_WIDTH 350

#define WIDGET_SCALE 3
#define BUTTON_WIDTH (ROSE_ASCII_WIDTH * 3)
#define BUTTON_LARGE_WIDTH (ROSE_ASCII_WIDTH * 5)
#define BUTTON_HEIGHT ((ROSE_ASCII_HEIGHT + 4) * WIDGET_SCALE)

#define BUTTON_PADDING 20
#define BUTTON_XPOS(x) (BUTTON_PADDING + (((BUTTON_WIDTH * WIDGET_SCALE) + BUTTON_PADDING) * x))
#define BUTTON_LARGE_XPOS(x) (BUTTON_PADDING + (((BUTTON_LARGE_WIDTH * WIDGET_SCALE) + BUTTON_PADDING) * x))
#define BUTTON_YPOS(x) (BUTTON_PADDING + ((BUTTON_HEIGHT + BUTTON_PADDING) * x))
#define SLIDER_WIDTH 48

typedef struct Map {
	uint32_t width;
	float sea_level;
	ROSE_Image image;
	ROSE_Perlin perlin;
	ROSE_Sprite sprite;

	bool show_raw_noise;
	bool show_clipped_noise;

} Map;

void MapUpdate(Map* map)
{
	ROSE_ImageDestroy(&map->image);
	ROSE_PerlinGenerate(&map->perlin);

	if (map->show_raw_noise) {
		map->image = ROSE_ImageCreateFromPerlin(&map->perlin);
		ROSE_SpriteUpdateTexture(&map->sprite, &map->image);
		ROSE_ImageDestroy(&map->image);
		return;
	}

	int32_t width = map->width;
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

	if (map->show_clipped_noise) {
		map->image = ROSE_ImageCreateFromPerlin(&map->perlin);
		ROSE_SpriteUpdateTexture(&map->sprite, &map->image);
		ROSE_ImageDestroy(&map->image);
		return;
	}

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
				ROSE_ImageSetPixelColor(&map->image, x, y, ROSE_COLOR_GREEN);
			}
			else {
				ROSE_ImageSetPixelColor(&map->image, x, y, ROSE_COLOR_BLUE);
			}
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
	map.perlin.attenuation = 0.7f;
	MapUpdate(&map);

	// [  SAVE MAP  ]

	ROSE_Button button_save_map      = ROSE_ButtonCreate("SAVE MAP");
	ROSE_Button button_random_seed   = ROSE_ButtonCreate("RANDOM SEED");
	
	ROSE_Button button_init_divs_dec = ROSE_ButtonCreate("-");
	ROSE_Button button_init_divs_inc = ROSE_ButtonCreate("+");
	ROSE_Text   text_init_divs       = ROSE_TextCreate("SCALE:     %d", map.perlin.divisions);
	
	ROSE_Button button_layers_dec    = ROSE_ButtonCreate("-");
	ROSE_Button button_layers_inc    = ROSE_ButtonCreate("+");
	ROSE_Text   text_layers          = ROSE_TextCreate("LAYERS:    %d", map.perlin.layers);
	
	ROSE_Slider slider_attenuation   = ROSE_SliderCreate();
	ROSE_Text   text_attenuation     = ROSE_TextCreate("DETAIL:    %.2f", map.perlin.attenuation);
	
	ROSE_Slider slider_sea_level     = ROSE_SliderCreate();
	ROSE_Text   text_sea_level       = ROSE_TextCreate("SEA LEVEL: %.2f", map.sea_level);
	
	ROSE_Button button_show_raw      = ROSE_ButtonCreate("RAW");
	ROSE_Button button_show_clipped  = ROSE_ButtonCreate("CLIP");
	ROSE_Button button_show_land     = ROSE_ButtonCreate("LAND");
	
	slider_attenuation.subject = &map.perlin.attenuation;
	slider_sea_level.subject   = &map.sea_level;

	button_save_map.scale      = WIDGET_SCALE;
	button_random_seed.scale   = WIDGET_SCALE;
	button_init_divs_dec.scale = WIDGET_SCALE;
	button_init_divs_inc.scale = WIDGET_SCALE;
	text_init_divs.scale       = WIDGET_SCALE;
	button_layers_dec.scale    = WIDGET_SCALE;
	button_layers_inc.scale    = WIDGET_SCALE;
	text_layers.scale          = WIDGET_SCALE;
	slider_attenuation.scale   = WIDGET_SCALE;
	text_attenuation.scale     = WIDGET_SCALE;
	slider_sea_level.scale     = WIDGET_SCALE;
	text_sea_level.scale       = WIDGET_SCALE;
	button_show_raw.scale      = WIDGET_SCALE;
	button_show_clipped.scale  = WIDGET_SCALE;
	button_show_land.scale     = WIDGET_SCALE;

	button_save_map.xpos      = BUTTON_XPOS(0);
	button_random_seed.xpos   = BUTTON_XPOS(0);
	button_init_divs_dec.xpos = BUTTON_XPOS(0);
	button_init_divs_inc.xpos = BUTTON_XPOS(1);
	text_init_divs.xpos       = BUTTON_XPOS(2);
	button_layers_dec.xpos    = BUTTON_XPOS(0);
	button_layers_inc.xpos    = BUTTON_XPOS(1);
	text_layers.xpos          = BUTTON_XPOS(2);
	slider_attenuation.xpos   = BUTTON_XPOS(0);
	text_attenuation.xpos     = BUTTON_XPOS(2);
	slider_sea_level.xpos     = BUTTON_XPOS(0);
	text_sea_level.xpos       = BUTTON_XPOS(2);

	button_save_map.ypos      = BUTTON_YPOS(0);
	button_random_seed.ypos   = BUTTON_YPOS(1);
	button_init_divs_dec.ypos = BUTTON_YPOS(2);
	button_init_divs_inc.ypos = BUTTON_YPOS(2);
	text_init_divs.ypos       = BUTTON_YPOS(2) + (2 * WIDGET_SCALE);
	button_layers_dec.ypos    = BUTTON_YPOS(3);
	button_layers_inc.ypos    = BUTTON_YPOS(3);
	text_layers.ypos          = BUTTON_YPOS(3) + (2 * WIDGET_SCALE);
	slider_attenuation.ypos   = BUTTON_YPOS(4);
	text_attenuation.ypos     = BUTTON_YPOS(4) + (2 * WIDGET_SCALE);
	slider_sea_level.ypos     = BUTTON_YPOS(5);
	text_sea_level.ypos       = BUTTON_YPOS(5) + (2 * WIDGET_SCALE);
	
	button_init_divs_dec.width = BUTTON_WIDTH;
	button_init_divs_inc.width = BUTTON_WIDTH;
	button_layers_dec.width    = BUTTON_WIDTH;
	button_layers_inc.width    = BUTTON_WIDTH;
	slider_attenuation.width   = SLIDER_WIDTH;
	slider_sea_level.width     = SLIDER_WIDTH;
	button_show_raw.width      = BUTTON_LARGE_WIDTH;
	button_show_clipped.width  = BUTTON_LARGE_WIDTH;
	button_show_land.width     = BUTTON_LARGE_WIDTH;

	while (ROSE_WindowPollEvents())
	{
		if (ROSE_ButtonPressed(&button_random_seed)) {
			map.perlin.seed = rand();
		}

		if (ROSE_ButtonPressed(&button_init_divs_inc)) {
			if (map.perlin.divisions < 20) {
				map.perlin.divisions++;
				ROSE_TextUpdateString(&text_init_divs, "SCALE:  %d", map.perlin.divisions);
			}
		}

		if (ROSE_ButtonPressed(&button_init_divs_dec)) {
			if (map.perlin.divisions > 1) {
				map.perlin.divisions--;
				ROSE_TextUpdateString(&text_init_divs, "SCALE:  %d", map.perlin.divisions);
			}
		}

		if (ROSE_ButtonPressed(&button_layers_inc)) {
			if (map.perlin.layers < 8) {
				map.perlin.layers++;
				ROSE_TextUpdateString(&text_layers, "LAYERS: %d", map.perlin.layers);
			}
		}

		if (ROSE_ButtonPressed(&button_layers_dec)) {
			if (map.perlin.layers > 1) {
				map.perlin.layers--;
				ROSE_TextUpdateString(&text_layers, "LAYERS: %d", map.perlin.layers);
			}
		}

		if (ROSE_ButtonPressed(&button_show_raw)) {
			map.show_raw_noise = true;
			map.show_clipped_noise = false;
		}

		if (ROSE_ButtonPressed(&button_show_clipped)) {
			map.show_raw_noise = false;
			map.show_clipped_noise = true;
		}

		if (ROSE_ButtonPressed(&button_show_land)) {
			map.show_raw_noise = false;
			map.show_clipped_noise = false;
		}

		if (ROSE_ButtonPressed(&button_save_map)) {
			ROSE_ImageSavePNG(&map.image, "map.png");
		}

		MapUpdate(&map);
		ROSE_WindowBeginFrame(ROSE_COLOR_BLACK);
		{
			const uint32_t MAP_PADDING = 50;
			map.sprite.xpos = rose_screen.width - (MAP_PADDING + (map.width * map.sprite.scale));
			map.sprite.ypos = MAP_PADDING;

			button_show_raw.xpos = BUTTON_LARGE_XPOS(0) + map.sprite.xpos;
			button_show_raw.ypos = BUTTON_YPOS(0) + map.sprite.ypos + (map.width * map.sprite.scale);

			button_show_clipped.xpos = BUTTON_LARGE_XPOS(1) + map.sprite.xpos;
			button_show_clipped.ypos = BUTTON_YPOS(0) + map.sprite.ypos + (map.width * map.sprite.scale);

			button_show_land.xpos = BUTTON_LARGE_XPOS(2) + map.sprite.xpos;
			button_show_land.ypos = BUTTON_YPOS(0) + map.sprite.ypos + (map.width * map.sprite.scale);

			ROSE_SpriteDraw(&map.sprite);

			ROSE_ButtonDraw(&button_save_map);
			ROSE_ButtonDraw(&button_random_seed);

			ROSE_ButtonDraw(&button_init_divs_dec);
			ROSE_ButtonDraw(&button_init_divs_inc);
			ROSE_TextDraw(&text_init_divs);

			ROSE_ButtonDraw(&button_layers_dec);
			ROSE_ButtonDraw(&button_layers_inc);
			ROSE_TextDraw(&text_layers);

			ROSE_SliderDraw(&slider_attenuation);
			ROSE_TextUpdateString(&text_attenuation, "DETAIL: %.2f", map.perlin.attenuation);
			ROSE_TextDraw(&text_attenuation);

			ROSE_SliderDraw(&slider_sea_level);
			ROSE_TextUpdateString(&text_sea_level, "SEA LEVEL: %.2f", map.sea_level);
			ROSE_TextDraw(&text_sea_level);

			ROSE_ButtonDraw(&button_show_raw);
			ROSE_ButtonDraw(&button_show_clipped);
			ROSE_ButtonDraw(&button_show_land);
		}
		ROSE_WindowEndFrame();
	}

	ROSE_Quit();
	return EXIT_SUCCESS;
}

