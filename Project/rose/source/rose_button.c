
/*
 *   Source File [rose_button.c]
 */

#define ROSE_EXTERN
#include "rose.h"

ROSE_Button ROSE_ButtonCreate(const char* string)
{
	ROSE_Button button = ROSE_ButtonCreateEmpty();
	ROSE_ButtonUpdateString(&button, string);
	ROSE__ButtonUpdate(&button);
	return button;
}

ROSE_Button ROSE_ButtonCreateEmpty(void)
{
	return (ROSE_Button) {
		.scale = 1,
		.xpos = 0,
		.ypos = 0,
		.width = 100,
		._width = 0,
		._curr_pressed = false,
		._prev_pressed = false,
		._sprite_normal = ROSE_SpriteCreateEmpty(0, 0),
		._sprite_pressed = ROSE_SpriteCreateEmpty(0, 0),
		._text = ROSE_TextCreateEmpty(),
	};
}

void ROSE_ButtonDestroy(ROSE_Button* button)
{
	if (button) {
		ROSE_SpriteDestroy(&button->_sprite_normal);
		ROSE_SpriteDestroy(&button->_sprite_pressed);
		ROSE_TextDestroy(&button->_text);
		*button = (ROSE_Button) { 0 };
	}
}

void ROSE_ButtonUpdateString(ROSE_Button* button, const char* string)
{
	ROSE_TextUpdateString(&button->_text, string);
}

void ROSE_ButtonDraw(ROSE_Button* button)
{
	button->_prev_pressed = button->_curr_pressed;
	if (rose_screen.minimized) return;
	if (!rose__render_pass) abort();

	ROSE__ButtonUpdate(button);
	const uint32_t width = button->_width;
	const uint32_t height = (ROSE_ASCII_HEIGHT + 4);

	const bool horizontal = 
		(((float)button->xpos <= rose_mouse.xpos) &&
		(rose_mouse.xpos < ((float)button->xpos + ((float)width * button->scale))));
	
	const bool vertical =
		(((float)button->ypos <= rose_mouse.ypos) &&
		(rose_mouse.ypos < ((float)button->ypos + ((float)height * button->scale))));
	
	const bool mouse_over = horizontal && vertical;
	
	if (ROSE_MousePressed(ROSE_BUTTON_LEFT) && (mouse_over))
		button->_curr_pressed = true;
	
	if (ROSE_MouseReleased(ROSE_BUTTON_LEFT) || (!mouse_over))
		button->_curr_pressed = false;
	
	if (button->_curr_pressed) {
		ROSE_SpriteDraw(&button->_sprite_pressed);
		button->_text.color.a = 0.5f;
		ROSE_TextDraw(&button->_text);
	} else {
		ROSE_SpriteDraw(&button->_sprite_normal);
		button->_text.color.a = 1.0f;
		ROSE_TextDraw(&button->_text);
	}
}

bool ROSE_ButtonPressed(ROSE_Button* button)
{
	return (!button->_prev_pressed) && (button->_curr_pressed);
}

bool ROSE_ButtonReleased(ROSE_Button* button)
{
	return (button->_prev_pressed) && (!button->_curr_pressed);
}

bool ROSE_ButtonPressing(ROSE_Button* button)
{
	return (button->_curr_pressed);
}

void ROSE__ButtonUpdate(ROSE_Button* button)
{
	button->_sprite_normal.scale = button->scale;
	button->_sprite_pressed.scale = button->scale;
	button->_text.scale = button->scale;

	if (button->_width != button->width)
	{
		button->_width = button->width;
		const uint32_t width = button->_width;
		const uint32_t height = (ROSE_ASCII_HEIGHT + 4);

		ROSE_Image image_normal  = ROSE_ImageCreate(width, height);
		ROSE_Image image_pressed = ROSE_ImageCreate(width, height);

		const ROSE_RGBA color_normal =             (ROSE_RGBA) { 100, 100, 100, 255 };
		const ROSE_RGBA color_pressed =            (ROSE_RGBA) {  80,  80,  80, 255 };
		const ROSE_RGBA color_dark_edge =          (ROSE_RGBA) {  70,  70,  70, 255 };
		const ROSE_RGBA color_light_edge_normal =  (ROSE_RGBA) { 135, 135, 135, 255 };
		const ROSE_RGBA color_light_edge_pressed = (ROSE_RGBA) { 120, 120, 120, 255 };

		for (uint32_t x = 0; x < width; x++) {
			for (uint32_t y = 0; y < height; y++) {
				ROSE_ImageSetPixelRGBA(&image_normal, x, y, color_normal);
				ROSE_ImageSetPixelRGBA(&image_pressed, x, y, color_pressed);
			}
		}

		for (uint32_t y = 0; y < height; y++) {
			ROSE_ImageSetPixelRGBA(&image_normal, (width - 1), y, color_dark_edge);
			ROSE_ImageSetPixelRGBA(&image_normal, 0, y, color_light_edge_normal);
			ROSE_ImageSetPixelRGBA(&image_pressed, (width - 1), y, color_light_edge_pressed);
			ROSE_ImageSetPixelRGBA(&image_pressed, 0, y, color_dark_edge);
		}

		for (uint32_t x = 0; x < width; x++) {
			ROSE_ImageSetPixelRGBA(&image_normal, x, (height - 1), color_dark_edge);
			ROSE_ImageSetPixelRGBA(&image_normal, x, 0, color_light_edge_normal);
			ROSE_ImageSetPixelRGBA(&image_pressed, x, (height - 1), color_light_edge_pressed);
			ROSE_ImageSetPixelRGBA(&image_pressed, x, 0, color_dark_edge);
		}

		ROSE_SpriteUpdateTexture(&button->_sprite_normal, &image_normal);
		ROSE_SpriteUpdateTexture(&button->_sprite_pressed, &image_pressed);

		ROSE_ImageDestroy(&image_normal);
		ROSE_ImageDestroy(&image_pressed);
	}

	button->_sprite_normal.xpos = button->xpos;
	button->_sprite_normal.ypos = button->ypos;
	button->_sprite_pressed.xpos = button->xpos;
	button->_sprite_pressed.ypos = button->ypos;
	uint32_t button_width = button->_width * button->scale;
	uint32_t text_width = button->_text._num_characters * ROSE_ASCII_WIDTH * button->scale;
	uint32_t stride = (button_width > text_width) ? (button_width - text_width) : 0;
	button->_text.xpos = button->xpos + (int32_t)(stride / 2);
	button->_text.ypos = button->ypos + (int32_t)(2 * button->scale);
}

