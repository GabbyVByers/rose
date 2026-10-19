
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

    ROSE_Vertex2D triangle_vertices[] = {
        { {  0.0f,  0.5f } },
        { { -0.5f, -0.5f } },
        { {  0.5f, -0.5f } },
    };

    ROSE_Triangles triangle = ROSE_TrianglesCreate(triangle_vertices, sizeof(triangle_vertices) / sizeof(triangle_vertices[0]));
    triangle.color = ROSE_COLOR_PURPLE;

    ROSE_Vertex2D point_vertices[] = {
        { {  0.0f,  0.5f } },
        { { -0.5f, -0.5f } },
        { {  0.5f, -0.5f } },
        { {  0.0f,  0.0f } },
        { { -1.0f,  0.8f } },
        { {  1.0f,  0.8f } },
    };

    ROSE_Points points = ROSE_PointsCreate(point_vertices, sizeof(point_vertices) / sizeof(point_vertices[0]));
    points.color = ROSE_COLOR_YELLOW;
    points.diameter = 16.0;

    ROSE_Vertex2D line_vertices[] = {
        { {  0.0f,  0.7f } },
        { { -0.7f, -0.7f } },
        { {  0.7f, -0.7f } },
        { {  0.0f,  0.0f } },
        { { -0.8f,  0.4f } },
        { {  0.8f,  0.4f } },
    };

    ROSE_Lines lines = ROSE_LinesCreate(line_vertices, sizeof(line_vertices) / sizeof(line_vertices[0]));
    lines.color = ROSE_COLOR_RED;

    while (ROSE_WindowPollEvents()) {
        sprite.xpos = rose_mouse.xpos;
        sprite.ypos = rose_mouse.ypos;
        ROSE_WindowClear(ROSE_COLOR(50, 50, 50, 255));
        ROSE_TrianglesDraw(&triangle);
        ROSE_PointsDraw(&points);
        ROSE_TextDraw(&text);
        ROSE_SpriteDraw(&sprite);
        ROSE_LinesDraw(&lines);
        ROSE_WindowRender();
    }

    ROSE_ImageDestroy(&image);
    ROSE_TextDestroy(&text);
    ROSE_SpriteDestroy(&sprite);
    ROSE_TrianglesDestroy(&triangle);
    ROSE_PointsDestroy(&points);
    ROSE_LinesDestroy(&lines);
    ROSE_Quit();
}

