
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

typedef struct Map {
	uint32_t width;
	ROSE_PerlinInfo info;
	ROSE_Image image;
	ROSE_Sprite sprite;
} Map;

void MapUpdate(Map* map)
{
	ROSE_ImageDestroy(&map->image);
	map->image = ROSE_ImageCreatePerlin(map->width, map->width, &map->info);
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
		.layers = 1,
		.attenuation = 0.7f,
		.division_growth = 2.0f,
	};

	map.width = 700;
	map.image = ROSE_ImageCreatePerlin(map.width, map.width, &map.info);
	map.sprite = ROSE_SpriteCreate(&map.image);
	map.sprite.xpos = 500;
	map.sprite.ypos = 100;

	// [  RANDOM SEED  ]

	ROSE_Button random_seed_button = ROSE_ButtonCreate("RANDOM SEED");
	random_seed_button.scale = BUTTON_SCALE;
	random_seed_button.xpos = BUTTON_XPOS(0);
	random_seed_button.ypos = BUTTON_YPOS(0);
	
	// [ - ] [ + ] DIVISIONS [#]

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

	ROSE_Text init_divs_text = ROSE_TextCreate("DIVISIONS [%d]", map.info.divisions);
	init_divs_text.scale = BUTTON_SCALE;
	init_divs_text.xpos = BUTTON_XPOS(2);
	init_divs_text.ypos = TEXT_YPOS(1);

	// [ - ] [ + ] LAYERS [#]

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

	ROSE_Text layers_text = ROSE_TextCreate("LAYERS [%d]", map.info.layers);
	layers_text.scale = BUTTON_SCALE;
	layers_text.xpos = BUTTON_XPOS(2);
	layers_text.ypos = TEXT_YPOS(2);

	// --- [SLIDER] ------------------------------------------------------------------------------
	float subject = 0.5f;
	ROSE_Slider slider = ROSE_SliderCreate();
	slider.subject = &subject;
	slider.scale = 3;
	slider.xpos = 350;
	slider.ypos = 400;
	slider.min = -10.0f;
	slider.max = 30.0f;
	slider.width = 190;

	while (ROSE_WindowPollEvents())
	{
		if (ROSE_ButtonPressed(&random_seed_button)) {
			map.info.seed = rand();
			MapUpdate(&map);
		}

		if (ROSE_ButtonPressed(&init_divs_inc_button)) {
			if (map.info.divisions < 20) {
				map.info.divisions++;
				ROSE_TextUpdateString(&init_divs_text, "DIVISIONS [%d]", map.info.divisions);
				MapUpdate(&map);
			}
		}

		if (ROSE_ButtonPressed(&init_divs_dec_button)) {
			if (map.info.divisions > 1) {
				map.info.divisions--;
				ROSE_TextUpdateString(&init_divs_text, "DIVISIONS [%d]", map.info.divisions);
				MapUpdate(&map);
			}
		}

		if (ROSE_ButtonPressed(&layers_inc_button)) {
			if (map.info.layers < 8) {
				map.info.layers++;
				ROSE_TextUpdateString(&layers_text, "LAYERS [%d]", map.info.layers);
				MapUpdate(&map);
			}
		}

		if (ROSE_ButtonPressed(&layers_dec_button)) {
			if (map.info.layers > 1) {
				map.info.layers--;
				ROSE_TextUpdateString(&layers_text, "LAYERS [%d]", map.info.layers);
				MapUpdate(&map);
			}
		}

		ROSE_WindowBeginFrame(ROSE_COLOR_BLACK);
		{
			const uint32_t MAP_PADDING = 50;
			map.sprite.xpos = rose_screen.width - (MAP_PADDING + map.width);
			map.sprite.ypos = MAP_PADDING;

			ROSE_SpriteDraw(&map.sprite);
			ROSE_ButtonDraw(&random_seed_button);

			ROSE_ButtonDraw(&init_divs_dec_button);
			ROSE_ButtonDraw(&init_divs_inc_button);
			ROSE_TextDraw(&init_divs_text);

			ROSE_ButtonDraw(&layers_dec_button);
			ROSE_ButtonDraw(&layers_inc_button);
			ROSE_TextDraw(&layers_text);

			// --- [SLIDER] ------------------------------------------------------------------------------
			ROSE_SliderDraw(&slider);
			if (ROSE_SliderReleased(&slider)) {
				fprintf(stdout, "Slider Released!\n");
			}
		}
		ROSE_WindowEndFrame();
	}

	ROSE_Quit();
	return EXIT_SUCCESS;
}

