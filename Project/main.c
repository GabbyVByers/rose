
/*
 *   Source File [main.c]
 */

#include "rose.h"

extern float mouse_xpos;
extern float mouse_ypos;

int main(void)
{
    ROSE_Init("App Title", 1200, 700, true);

    ROSE_Text text = ROSE_TextCreate("HELLO!");
    text.scale = 3.0;
    text.color = (ROSE_Color){ 1,0,0,1 };

    ROSE_Image image = ROSE_ImageLoadFromFile("textures/art.png");
    ROSE_Sprite sprite = ROSE_SpriteCreate(&image);

    while (ROSE_WindowIsOpen()) {
        sprite.xpos = mouse_xpos;
        sprite.ypos = mouse_ypos;
        ROSE_WindowClear((ROSE_Color) { 0, 0, 0, 1 });
        ROSE_TextDraw(&text);
        ROSE_SpriteDraw(&sprite);
        ROSE_WindowRender();
    }

    ROSE_Quit();
}

