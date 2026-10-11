
/*
 *   Source File [main.c]
 */

#include "rose.h"

int main(void)
{
    ROSE_Init("Title", 500, 500, true);
    ROSE_WindowToggleVSync(true);

    ROSE_Text* text = ROSE_TextCreate("HELLO!");
    ROSE_Image* image = ROSE_ImageCreate(100, 200);
    ROSE_Sprite* sprite = ROSE_SpriteCreate(image);

    while (ROSE_WindowIsOpen())
    {
        i32 mouse_x = ROSE_MousePositionX();
        i32 mouse_y = ROSE_MousePositionY();

        ROSE_WindowClear(ROSE_COLOR(80, 80, 80, 255));
        ROSE_SpriteDraw(sprite, 100, 100, 1.0f, ROSE_COLOR_WHITE);
        ROSE_TextDraw(text, mouse_x, mouse_y, 3.0f, ROSE_COLOR_PURPLE);
        ROSE_WindowRender();
    }

    ROSE_Quit();
    return EXIT_SUCCESS;
}

