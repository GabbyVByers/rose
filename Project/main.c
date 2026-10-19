
/*
 *   Source File [main.c]
 */

#include "rose.h"

extern ROSE_ScreenInfo rose_screen;
extern ROSE_MouseInfo  rose_mouse;

int main(void)
{
    ROSE_Init("App Title", 1200, 700, true);

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

    ROSE_Quit();
}

