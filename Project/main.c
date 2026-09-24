
/*
 *   Source File [main.c]
 */

#include "rose.h"

int main(void) {
	ROSE_Init("App", 1200, 700, TRUE);
	ROSE_WindowToggleVSync(TRUE);

	ROSE_Text* text = ROSE_TextCreate("hello!");

	ROSE_Image* image = ROSE_ImageCreate(50, 50);
	for (i32 x = 0; x < 50; x++) {
		for (i32 y = 0; y < 50; y++) {
			if (x != y) {
				ROSE_ImagePutPixel(image, x, y, ROSE_COLOR_CYAN);
			}
		}
	}

	ROSE_Sprite* sprite = ROSE_SpriteCreate(NULL);
	ROSE_SpriteUploadTexture(sprite, image);

	ROSE_Image* perlin = ROSE_ImagePerlinNoise(500, 9);
	ROSE_Sprite* perlin_sprite = ROSE_SpriteCreate(NULL);
	ROSE_SpriteUploadTexture(perlin_sprite, perlin);

	ROSE_ImageSaveAsPNG(perlin, "perlin.png");

	while (ROSE_WindowIsOpen()) {
		ROSE_WindowClearScreen((ROSE_Color){ 0.1f, 0.1f, 0.1f, 1.0f });

		i32 x, y;
		ROSE_MousePosition(&x, &y);

		ROSE_WindowDrawText(text, x, y, 5.0, ROSE_COLOR_RED);
		ROSE_WindowDrawSprite(sprite, 20, 20, 5.0, ROSE_COLOR_WHITE);
		ROSE_WindowDrawSprite(perlin_sprite, 500, 100, 1.0, ROSE_COLOR_WHITE);
		ROSE_WindowRender();
	}
	
	ROSE_TextDestroy(text);
	ROSE_ImageDestroy(image);
	ROSE_ImageDestroy(perlin);
	ROSE_SpriteDestroy(sprite);
	ROSE_SpriteDestroy(perlin_sprite);
	ROSE_Quit();
	return EXIT_SUCCESS;
}

