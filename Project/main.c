
/*
 *   Source File [main.c]
 */

#include "rose.h"

#define BUTTON_SCALE 3
#define BUTTON_WIDTH (ROSE_ASCII_WIDTH * 3)
#define BUTTON_HEIGHT ((ROSE_ASCII_HEIGHT + 4) * BUTTON_SCALE)

#define BUTTON_PADDING 20
#define BUTTON_XPOS(x) (BUTTON_PADDING + (((BUTTON_WIDTH * BUTTON_SCALE) + BUTTON_PADDING) * x))
#define BUTTON_YPOS(x) (BUTTON_PADDING + ((BUTTON_HEIGHT + BUTTON_PADDING) * x))
#define TEXT_YPOS(x) ((BUTTON_PADDING + ((BUTTON_HEIGHT + BUTTON_PADDING) * x)) + (2 * BUTTON_SCALE))
#define SLIDER_WIDTH 48

typedef struct Map {
	uint32_t width;
	ROSE_PerlinInfo info;
	ROSE_Image image;
	ROSE_Sprite sprite;
	float sea_level;
} Map;

void MapUpdate(Map* map)
{
	ROSE_ImageDestroy(&map->image);
	map->image = ROSE_ImageCreatePerlin(map->width, map->width, &map->info);
	
	int32_t radius = map->image._width / 2;
	int32_t width = map->image._width;

	for (int32_t y = 0; y < width; y++) {
		for (int32_t x = 0; x < width; x++) {
			int32_t dx = x - radius;
			int32_t dy = y - radius;
			int32_t dist2 = (dx * dx) + (dy * dy);
			int32_t radius2 = radius * radius;
			if (dist2 > (radius * radius)) {
				ROSE_ImageSetPixelRGBA(&map->image, x, y, (ROSE_RGBA) { 0, 0, 0, 255 });
			} else {
				ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&map->image, x, y);
				float strength = 1.0f - ((float)dist2 / radius2);
				color.r *= strength;
				color.g *= strength;
				color.b *= strength;
				ROSE_ImageSetPixelRGBA(&map->image, x, y, color);
			}
		}
	}

	for (int32_t y = 0; y < width; y++) {
		for (int32_t x = 0; x < width; x++) {
			ROSE_RGBA color = ROSE_ImageGetPixelRGBA(&map->image, x, y);
			float intensity = (float)color.r / 255.0f;
			if (intensity < map->sea_level) {
				ROSE_ImageSetPixelColor(&map->image, x, y, ROSE_COLOR_BLUE);
			} else {
				ROSE_ImageSetPixelColor(&map->image, x, y, ROSE_COLOR_GREEN);
			}
		}
	}

	ROSE_SpriteUpdateTexture(&map->sprite, &map->image);
}

int main(void)
{
	ROSE_Init("Pearl", 1600, 800);
	ROSE_WindowVSync(false);

	Map map = { 0 };
	map.info = (ROSE_PerlinInfo) {
		.seed = 0,
		.divisions = 5,
		.layers = 8,
		.attenuation = 1.0f,
		.division_growth = 2.0f,
	};

	map.width = 350;
	map.image = ROSE_ImageCreatePerlin(map.width, map.width, &map.info);
	map.sprite = ROSE_SpriteCreate(&map.image);
	map.sprite.scale = 2;
	map.sprite.xpos = 500;
	map.sprite.ypos = 100;
	map.sea_level = 0.5f;

	// [  RANDOM SEED  ]

	ROSE_Button random_seed_button = ROSE_ButtonCreate("RANDOM SEED");
	random_seed_button.scale = BUTTON_SCALE;
	random_seed_button.xpos = BUTTON_XPOS(0);
	random_seed_button.ypos = BUTTON_YPOS(0);
	
	// [ - ] [ + ] DIVISIONS

	ROSE_Button init_divs_dec_button = ROSE_ButtonCreate("-");
	init_divs_dec_button.scale = BUTTON_SCALE;
	init_divs_dec_button.width = BUTTON_WIDTH;
	init_divs_dec_button.xpos = BUTTON_XPOS(0);
	init_divs_dec_button.ypos = BUTTON_YPOS(1);

	ROSE_Button init_divs_inc_button = ROSE_ButtonCreate("+");
	init_divs_inc_button.scale = BUTTON_SCALE;
	init_divs_inc_button.width = BUTTON_WIDTH;
	init_divs_inc_button.xpos = BUTTON_XPOS(1);
	init_divs_inc_button.ypos = BUTTON_YPOS(1);

	ROSE_Text init_divs_text = ROSE_TextCreate("SCALE:  %d", map.info.divisions);
	init_divs_text.scale = BUTTON_SCALE;
	init_divs_text.xpos = BUTTON_XPOS(2);
	init_divs_text.ypos = TEXT_YPOS(1);

	// [ - ] [ + ] LAYERS

	ROSE_Button layers_dec_button = ROSE_ButtonCreate("-");
	layers_dec_button.scale = BUTTON_SCALE;
	layers_dec_button.width = BUTTON_WIDTH;
	layers_dec_button.xpos = BUTTON_XPOS(0);
	layers_dec_button.ypos = BUTTON_YPOS(2);

	ROSE_Button layers_inc_button = ROSE_ButtonCreate("+");
	layers_inc_button.scale = BUTTON_SCALE;
	layers_inc_button.width = BUTTON_WIDTH;
	layers_inc_button.xpos = BUTTON_XPOS(1);
	layers_inc_button.ypos = BUTTON_YPOS(2);

	ROSE_Text layers_text = ROSE_TextCreate("LAYERS: %d", map.info.layers);
	layers_text.scale = BUTTON_SCALE;
	layers_text.xpos = BUTTON_XPOS(2);
	layers_text.ypos = TEXT_YPOS(2);

	// --[]------ ATTENUATION

	ROSE_Slider attenuation_slider = ROSE_SliderCreate();
	attenuation_slider.subject = &map.info.attenuation;
	attenuation_slider.width = SLIDER_WIDTH;
	attenuation_slider.scale = BUTTON_SCALE;
	attenuation_slider.xpos = BUTTON_XPOS(0);
	attenuation_slider.ypos = BUTTON_YPOS(3);

	ROSE_Text attenuation_text = ROSE_TextCreate("DETAIL: %.2f", map.info.attenuation);
	attenuation_text.scale = BUTTON_SCALE;
	attenuation_text.xpos = BUTTON_XPOS(2);
	attenuation_text.ypos = TEXT_YPOS(3);

	// --[]------ SEA LEVEL

	ROSE_Slider sea_level_slider = ROSE_SliderCreate();
	sea_level_slider.subject = &map.sea_level;
	sea_level_slider.width = SLIDER_WIDTH;
	sea_level_slider.scale = BUTTON_SCALE;
	sea_level_slider.xpos = BUTTON_XPOS(0);
	sea_level_slider.ypos = BUTTON_YPOS(4);

	ROSE_Text sea_level_text = ROSE_TextCreate("SEA LEVEL: %.2f", map.sea_level);
	sea_level_text.scale = BUTTON_SCALE;
	sea_level_text.xpos = BUTTON_XPOS(2);
	sea_level_text.ypos = TEXT_YPOS(4);

	while (ROSE_WindowPollEvents())
	{
		if (ROSE_ButtonPressed(&random_seed_button)) {
			map.info.seed = rand();
			//MapUpdate(&map);
		}

		if (ROSE_ButtonPressed(&init_divs_inc_button)) {
			if (map.info.divisions < 20) {
				map.info.divisions++;
				ROSE_TextUpdateString(&init_divs_text, "SCALE:  %d", map.info.divisions);
				//MapUpdate(&map);
			}
		}

		if (ROSE_ButtonPressed(&init_divs_dec_button)) {
			if (map.info.divisions > 1) {
				map.info.divisions--;
				ROSE_TextUpdateString(&init_divs_text, "SCALE:  %d", map.info.divisions);
				//MapUpdate(&map);
			}
		}

		if (ROSE_ButtonPressed(&layers_inc_button)) {
			if (map.info.layers < 8) {
				map.info.layers++;
				ROSE_TextUpdateString(&layers_text, "LAYERS: %d", map.info.layers);
				//MapUpdate(&map);
			}
		}

		if (ROSE_ButtonPressed(&layers_dec_button)) {
			if (map.info.layers > 1) {
				map.info.layers--;
				ROSE_TextUpdateString(&layers_text, "LAYERS: %d", map.info.layers);
				//MapUpdate(&map);
			}
		}

		MapUpdate(&map);
		ROSE_WindowBeginFrame(ROSE_COLOR_BLACK);
		{
			const uint32_t MAP_PADDING = 50;
			map.sprite.xpos = rose_screen.width - (MAP_PADDING + (map.width * map.sprite.scale));
			map.sprite.ypos = MAP_PADDING;

			ROSE_SpriteDraw(&map.sprite);
			ROSE_ButtonDraw(&random_seed_button);

			ROSE_ButtonDraw(&init_divs_dec_button);
			ROSE_ButtonDraw(&init_divs_inc_button);
			ROSE_TextDraw(&init_divs_text);

			ROSE_ButtonDraw(&layers_dec_button);
			ROSE_ButtonDraw(&layers_inc_button);
			ROSE_TextDraw(&layers_text);

			ROSE_SliderDraw(&attenuation_slider);
			ROSE_TextUpdateString(&attenuation_text, "DETAIL: %.2f", map.info.attenuation);
			ROSE_TextDraw(&attenuation_text);

			ROSE_SliderDraw(&sea_level_slider);
			ROSE_TextUpdateString(&sea_level_text, "SEA LEVEL: %.2f", map.sea_level);
			ROSE_TextDraw(&sea_level_text);
		}
		ROSE_WindowEndFrame();
	}

	ROSE_Quit();
	return EXIT_SUCCESS;
}

