
/*
 *   Source File [main.c]
 */

#include "rose.h"

int main(void)
{
    ROSE_Init("App Title", 1200, 700);

    ROSE_Text text = ROSE_TextCreate("HELLO!");
    text.scale = 3.0;
    text.color = ROSE_COLOR_RED;

    ROSE_TextUpdateString(&text, "!OLLEH");
    ROSE_TextUpdateString(&text, "!OLLEHFFFFF");

    ROSE_Image image = ROSE_ImageLoadFromFile("textures/art.png");
    ROSE_Sprite sprite = ROSE_SpriteCreate(&image);

    while (ROSE_WindowPollEvents()) {
        sprite.xpos = rose_mouse.xpos;
        sprite.ypos = rose_mouse.ypos;
        ROSE_WindowClear(ROSE_COLOR(50, 50, 50, 255));
        ROSE_TextDraw(&text);
        ROSE_SpriteDraw(&sprite);
        ROSE_WindowRender();
    }

    ROSE_ImageDestroy(&image);
    ROSE_TextDestroy(&text);
    ROSE_SpriteDestroy(&sprite);
    ROSE_Quit();
}

