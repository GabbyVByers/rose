
/*
 *   Source File [main.c]
 */

#include "rose.h"

int main(void)
{
    ROSE_Init("App Title", 1200, 700);

    ROSE_Button button = ROSE_ButtonCreate("HELLO");
    button.xpos = 300;
    button.ypos = 20;
    button.scale = 3;
    ROSE_ButtonUpdateString(&button, "haufkjdlhsafjkldhsajkflhdsjaklfhjasdkl");

    while (ROSE_WindowPollEvents()) {
        ROSE_WindowBeginFrame(ROSE_COLOR_BLACK);
        ROSE_ButtonDraw(&button);
        ROSE_WindowEndFrame();
    }

    ROSE_ButtonDestroy(&button);
    ROSE_Quit();
}

