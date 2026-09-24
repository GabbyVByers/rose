
/*
 *   Source File [main.c]
 */

#include "rose.h"

extern i32 screen_width;
extern i32 screen_height;

int main(void) {
	ROSE_Init("App", 1200, 700, TRUE);
	ROSE_WindowToggleVSync(TRUE);

	double zoom = 1.0;
	i32 px = 100;
	i32 py = 100;

	ROSE_Image* canvas_image = ROSE_ImageCreate(100, 100);

	ROSE_Sprite* canvas_sprite = ROSE_SpriteCreate(NULL);
	ROSE_SpriteUploadTexture(canvas_sprite, canvas_image);

	// save button
	ROSE_Text* save_text = ROSE_TextCreate("SAVE");
	ROSE_Sprite* save_up = ROSE_SpriteCreate("textures/button_up.png");
	ROSE_Sprite* save_down = ROSE_SpriteCreate("textures/button_down.png");

	while (ROSE_WindowIsOpen()) {
		i32 mouse_x, mouse_y;
		ROSE_MousePosition(&mouse_x, &mouse_y);
		
		ROSE_WindowClearScreen(ROSE_COLOR_BLACK);
		ROSE_WindowDrawSprite(canvas_sprite, px, py, zoom, ROSE_COLOR_WHITE);
		
		{ // SAVE BUTTON
			i32 w = save_up->width;
			i32 h = save_up->height;
			i32 x = 100;
			i32 y = screen_height - 100;
			ROSE_WindowDrawSprite(save_up, x, y, 1.0, ROSE_COLOR_WHITE);
			ROSE_WindowDrawText(save_text, x + w, y, 2.0, ROSE_COLOR_WHITE);
			
			if (((x <= mouse_x) && (mouse_x < (x + h))) && ((y <= mouse_y) && (mouse_y < (y + h)))) {
				if (ROSE_MousePressing(ROSE_MOUSE_LEFT)) {
					ROSE_WindowDrawSprite(save_down, x, y, 1.0, ROSE_COLOR_WHITE);
				}
				if (ROSE_MousePressed(ROSE_MOUSE_LEFT)) {
					ROSE_ImageSaveAsPNG(canvas_image, "untitled.png");
				}
			}
		}

		ROSE_WindowRender();
	}
	
	ROSE_Quit();
	return EXIT_SUCCESS;
}

