
/*
 *   Source File [rose_button.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

ROSE_Button ROSE_ButtonCreate(uint32_t width, const char* string)
{
	ROSE_Button button = ROSE_ButtonCreateEmpty();
	ROSE_ButtonUpdateString(&button, string);
	ROSE_ButtonUpdateWidth(&button, width);
	return button;
}

ROSE_Button ROSE_ButtonCreateEmpty(void)
{
	return (ROSE_Button) {
		.scale = 1.0,
		.xpos = 0.0,
		.ypos = 0.0,
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

void ROSE_ButtonUpdateWidth(ROSE_Button* button, uint32_t width)
{
	ROSE_Image image_normal = ROSE_ImageCreate(width, 15);
	ROSE_Image image_pressed = ROSE_ImageCreate(width, 15);
	button->_width = width;

	const ROSE_Color color_normal     = ROSE_COLOR(100, 100, 100, 255);
	const ROSE_Color color_pressed    = ROSE_COLOR( 80,  80,  80, 255);
	const ROSE_Color color_dark_edge  = ROSE_COLOR( 70,  70,  70, 255);
	const ROSE_Color color_light_edge = ROSE_COLOR(135, 135, 135, 255);

	for (uint32_t x = 0; x < width; x++) {
		for (uint32_t y = 0; y < 15; y++) {
			ROSE_ImageSetPixelColor(&image_normal, x, y, color_normal);
			ROSE_ImageSetPixelColor(&image_pressed, x, y, color_pressed);
		}
	}

	for (uint32_t y = 0; y < 15; y++) {
		ROSE_ImageSetPixelColor(&image_normal, (width - 1), y, color_dark_edge);
		ROSE_ImageSetPixelColor(&image_normal, 0, y, color_light_edge);
		ROSE_ImageSetPixelColor(&image_pressed, (width - 1), y, color_light_edge);
		ROSE_ImageSetPixelColor(&image_pressed, 0, y, color_dark_edge);
	}

	for (uint32_t x = 0; x < width; x++) {
		ROSE_ImageSetPixelColor(&image_normal, x, 14, color_dark_edge);
		ROSE_ImageSetPixelColor(&image_normal, x, 0, color_light_edge);
		ROSE_ImageSetPixelColor(&image_pressed, x, 14, color_light_edge);
		ROSE_ImageSetPixelColor(&image_pressed, x, 0, color_dark_edge);
	}

	ROSE_SpriteUpdateTexture(&button->_sprite_normal, &image_normal);
	ROSE_SpriteUpdateTexture(&button->_sprite_pressed, &image_pressed);

	ROSE_ImageDestroy(&image_normal);
	ROSE_ImageDestroy(&image_pressed);
}

void ROSE_ButtonDraw(ROSE_Button* button)
{
	ROSE__ButtonSetScale(button, button->scale);
	ROSE__ButtonSetPosition(button, button->xpos, button->ypos);

	const bool horizontal = 
		((button->xpos <= (double)rose_mouse.xpos) &&
		((double)rose_mouse.xpos <= (button->xpos + ((double)button->_width * button->scale))));
	
	const bool vertical =
		((button->ypos <= (double)rose_mouse.ypos) &&
		((double)rose_mouse.ypos <= (button->ypos + (15.0 * button->scale))));
	
	const bool mouse_over = horizontal && vertical;
	button->_prev_pressed = button->_curr_pressed;
	if (ROSE_MousePressed(ROSE_BUTTON_LEFT) && (mouse_over)) button->_curr_pressed = true;
	if (ROSE_MouseReleased(ROSE_BUTTON_LEFT) || (!mouse_over)) button->_curr_pressed = false;
	
	if (button->_curr_pressed) {
		ROSE_SpriteDraw(&button->_sprite_pressed);
		button->_text.color.a = 0.5f;
		ROSE_TextDraw(&button->_text);
	}
		
	else {
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

void ROSE__ButtonSetScale(ROSE_Button* button, double scale)
{
	button->_sprite_normal.scale = scale;
	button->_sprite_pressed.scale = scale;
	button->_text.scale = scale;
}

void ROSE__ButtonSetPosition(ROSE_Button* button, double xpos, double ypos)
{
	button->_sprite_normal.xpos = xpos;
	button->_sprite_normal.ypos = ypos;
	button->_sprite_pressed.xpos = xpos;
	button->_sprite_pressed.ypos = ypos;
	button->_text.xpos = xpos
		+ (0.5 * ((double)button->_width * button->scale))
		- (0.5 * ((double)button->_text._num_characters * (double)ROSE_ASCII_WIDTH * button->scale));
	button->_text.ypos = ypos + (2.0 * button->scale);
}

