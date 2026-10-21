
/*
 *   Source File [main.c]
 */

#include "rose.h"

static void ROSE__Uint32ToString(uint32_t number, char* string)
{
    uint32_t length = strlen(string);
    for (uint32_t i = (length - 1); i > 0; i--) {
        char character = (char)((number % 10) + 48);
        string[i] = character;
        number /= 10;
        if (number == 0)
            break;
    }
}

int main(void)
{
    ROSE_Init("App Title", 1200, 700);
    ROSE_WindowVSync(true);

    ROSE_Button button = ROSE_ButtonCreate("HELLO");
    button.xpos = 300;
    button.ypos = 20;
    button.scale = 3;
    ROSE_ButtonUpdateString(&button, "haufkjdlhsafjkldhsajkflhdsjaklfhjasdkl");

    ROSE_Vertex2D triangle_verts[] = {
        { 0.0, 0.0 },
        { 1.0, 0.0 },
        { 0.0, 1.0 },
        { 5.0, 5.0 },
        { 6.0, 5.0 },
        { 5.0, 6.0 },
    };

    ROSE_Mesh triangles = ROSE_MeshCreate(triangle_verts, sizeof(triangle_verts) / sizeof(triangle_verts[0]));
    triangles.shape = ROSE_LINES;
    triangles.scale = 0.1f;
    triangles.diameter = 3.0f;

    uint32_t number = 607;
    char* string = "xxxxx";

    ROSE__Uint32ToString(number, string);
    fprintf(stdout, string);

    ROSE_Text FPS = ROSE_TextCreate("FPS:");
    ROSE_Text framerate = ROSE_TextCreate("FPS:");

    while (ROSE_WindowPollEvents()) {
        ROSE_WindowBeginFrame(ROSE_COLOR_BLACK);
        ROSE_ButtonDraw(&button);
        ROSE_MeshDraw(&triangles);
        ROSE_WindowEndFrame();
    }

    ROSE_ButtonDestroy(&button);
    ROSE_MeshDestroy(&triangles);
    ROSE_Quit();
}

