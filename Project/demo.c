
/*
 *   Source File [main.c]
 */

#include "rose.h"

int main(void)
{
    ROSE_Init("App Title", 1200, 700);

    while (ROSE_WindowPollEvents()) {
        ROSE_WindowBeginFrame(ROSE_COLOR_BLACK);
        ROSE_WindowEndFrame();
    }

    ROSE_Quit();
}

