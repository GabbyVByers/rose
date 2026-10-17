
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

    ROSE_Vertex3D verts[] = {
        { { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
        { { 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
    };

    ROSE_Mesh* mesh = ROSE_MeshCreate();
    ROSE_MeshUploadVertices(mesh, verts, 6);

    while (ROSE_WindowIsOpen())
    {
        i32 mouse_x = ROSE_MousePositionX();
        i32 mouse_y = ROSE_MousePositionY();

        ROSE_WindowClear(ROSE_ColorROSE_COLOR(80, 80, 80, 255));
        ROSE_MeshDraw(mesh);
        ROSE_SpriteDraw(sprite, 100, 100, 1.0f, ROSE_ColorROSE_COLOR_WHITE);
        ROSE_TextDraw(text, mouse_x, mouse_y, 3.0f, ROSE_ColorROSE_COLOR_PURPLE);
        ROSE_WindowRender();
    }

    ROSE_Quit();
    return EXIT_SUCCESS;
}

