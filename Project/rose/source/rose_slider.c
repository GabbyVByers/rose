
/*
 *   Source File [rose_slider.c]
 */

#define ROSE_EXTERN
#include "rose.h"

ROSE_Slider ROSE_SliderCreate(void)
{
	uint32_t width = (ROSE_ASCII_WIDTH + 2);
	uint32_t height = (ROSE_ASCII_HEIGHT + 4);
	ROSE_Sprite sprite_handle_normal = ROSE_SpriteCreateGUIBox(width, height, true);
	ROSE_Sprite sprite_handle_pressed = ROSE_SpriteCreateGUIBox(width, height, false);

	return (ROSE_Slider) {
		.scale = 1,
		.xpos = 0,
		.ypos = 0,
		.width = 100,
		._width = 0,
		.subject = NULL,
		.min = 0.0f,
		.max = 1.0f,
		._sprite_handle_normal = sprite_handle_normal,
		._sprite_handle_pressed = sprite_handle_pressed,
		._sprite_bar = ROSE_SpriteCreateEmpty(1, 1),
	};
}

void ROSE_SliderDraw(ROSE_Slider* slider)
{
	if (slider->width != slider->_width) {
		slider->_width = slider->width;
		ROSE_SpriteDestroy(&slider->_sprite_bar);
		slider->_sprite_bar = ROSE_SpriteCreateGUIBox(slider->_width, 5, true);
	}

	float hand_xpos = 0.0f;
	float hand_width = (float)slider->_sprite_handle_normal._width * (float)slider->scale;
	float hand_height = (float)slider->_sprite_handle_normal._height * (float)slider->scale;

	{
		float mouse_x = rose_mouse.xpos;
		float bar_width = (float)slider->_width * (float)slider->scale;
		float travel_width = (bar_width > hand_width) ? (bar_width - hand_width) : 0.0f;
		float min = slider->min;
		float max = slider->max;

		if (slider->_curr_aquired) {
			float pos = mouse_x - (hand_width / 2.0f);
			float pos_min = slider->xpos;
			float pos_max = slider->xpos + travel_width;
			if (pos < pos_min)
				pos = pos_min;
			if (pos > pos_max)
				pos = pos_max;
			hand_xpos = pos;
			if (slider->subject && (travel_width > 0.0f))
				*slider->subject = min + ((max - min) * ((pos - slider->xpos) / travel_width));
		} else {
			float value = (slider->subject) ? (*slider->subject) : min;
			float travel = (max != min) ? ((value - min) / (max - min)) : 0.0f;
			hand_xpos = slider->xpos + (travel_width * travel);
		}
	}

	slider->_sprite_handle_normal.scale = slider->scale;
	slider->_sprite_handle_normal.xpos = hand_xpos;
	slider->_sprite_handle_normal.ypos = slider->ypos;

	slider->_sprite_handle_pressed.scale = slider->scale;
	slider->_sprite_handle_pressed.xpos = hand_xpos;
	slider->_sprite_handle_pressed.ypos = slider->ypos;

	slider->_sprite_bar.scale = slider->scale;
	slider->_sprite_bar.xpos = slider->xpos;
	slider->_sprite_bar.ypos = slider->ypos + (5 * slider->scale);

	const bool horizontal =
		(((float)slider->_sprite_handle_normal.xpos <= rose_mouse.xpos) &&
		(rose_mouse.xpos < ((float)slider->_sprite_handle_normal.xpos + hand_width)));

	const bool vertical =
		(((float)slider->ypos <= rose_mouse.ypos) &&
		(rose_mouse.ypos < ((float)slider->_sprite_handle_normal.ypos + hand_height)));

	const bool mouse_over = horizontal && vertical;

	slider->_prev_aquired = slider->_curr_aquired;
	if (mouse_over && ROSE_MousePressed(ROSE_BUTTON_LEFT))
		slider->_curr_aquired = true;
	if (ROSE_MouseReleased(ROSE_BUTTON_LEFT))
		slider->_curr_aquired = false;

	ROSE_SpriteDraw(&slider->_sprite_bar);
	if (slider->_curr_aquired) ROSE_SpriteDraw(&slider->_sprite_handle_pressed);
	else ROSE_SpriteDraw(&slider->_sprite_handle_normal);
}

bool ROSE_SliderPressed(ROSE_Slider* slider)
{
	return (!slider->_prev_aquired) && (slider->_curr_aquired);
}

bool ROSE_SliderReleased(ROSE_Slider* slider)
{
	return (slider->_prev_aquired) && (!slider->_curr_aquired);
}

bool ROSE_SliderPressing(ROSE_Slider* slider)
{
	return slider->_curr_aquired;
}

void ROSE_SliderDestroy(ROSE_Slider* slider)
{
	if (slider) {
		ROSE_SpriteDestroy(&slider->_sprite_handle_normal);
		ROSE_SpriteDestroy(&slider->_sprite_handle_pressed);
		ROSE_SpriteDestroy(&slider->_sprite_bar);
		*slider = (ROSE_Slider){ 0 };
	}
}

