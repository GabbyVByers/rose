
/*
 *   Source File [main.c]
 */

#include "rose.h"

#define PEARL_GAME
#if defined(PEARL_GAME)

int main(void)
{
    ROSE_Init("Pearl", 1200, 700);

    int x = 67;
    ROSE_Text text = ROSE_TextCreate("Hello: %d", x);
    text.scale = 3;
    text.xpos = 200;
    text.ypos = 200;

    while (ROSE_WindowPollEvents())
    {
        x++;
        ROSE_TextUpdateString(&text, "Hello: %d", x);

        ROSE_WindowBeginFrame(ROSE_COLOR_BLACK);
        ROSE_TextDraw(&text);
        ROSE_WindowEndFrame();
    }

    ROSE_Quit();
    return EXIT_SUCCESS;
}

#endif /* PEARL_GAME */

//#define DEMO_APPLICATION
#if defined(DEMO_APPLICATION)

static float randf32(void)
{
    return (((float)rand() / (float)RAND_MAX) * 2.0f) - 1.0f;
}

int main(void)
{
    ROSE_Init("App Title", 1200, 700);
    ROSE_WindowVSync(false);

    ROSE_Button button = ROSE_ButtonCreate("HELLO");
    button.xpos = 300;
    button.ypos = 20;
    button.scale = 3;

    ROSE_Vertex2D triangle_verts[128] = { 0 };
    for (uint32_t i = 0; i < 128; i++) {
        triangle_verts[i] = (ROSE_Vertex2D){ randf32(), randf32() };
    }
    
    ROSE_Vertex2D line_verts[128] = { 0 };
    for (uint32_t i = 0; i < 128; i++) {
        line_verts[i] = (ROSE_Vertex2D) { randf32(), randf32() };
    }
    
    ROSE_Vertex2D point_verts[128] = { 0 };
    for (uint32_t i = 0; i < 128; i++) {
        point_verts[i] = (ROSE_Vertex2D){ randf32(), randf32() };
    }

    ROSE_Mesh triangles = ROSE_MeshCreate(triangle_verts, sizeof(triangle_verts) / sizeof(triangle_verts[0]));
    triangles.shape = ROSE_TRIANGLES;
    triangles.color = ROSE_COLOR_CYAN;

    ROSE_Mesh lines = ROSE_MeshCreate(line_verts, sizeof(line_verts) / sizeof(line_verts[0]));
    lines.shape = ROSE_LINES;
    lines.color = ROSE_COLOR_PURPLE;

    ROSE_Mesh points = ROSE_MeshCreate(point_verts, sizeof(point_verts) / sizeof(point_verts[0]));
    points.shape = ROSE_POINTS;
    points.color = ROSE_COLOR_RED;
    points.diameter = 5.0f;

    ROSE_PerlinInfo info = {
        .seed = 0,
        .divisions = 1,
        .layers = 5,
        .attenuation = 0.5f,
        .division_scale = 2.0f,
    };

    ROSE_Image image = ROSE_ImageCreatePerlin(300, 300, &info);
    ROSE_Sprite sprite = ROSE_SpriteCreate(&image);
    sprite.xpos = 200;
    sprite.ypos = 200;

    uint32_t x = 0;
    uint32_t y = 0;

    while (ROSE_WindowPollEvents())
    {
        //ROSE_ImageSetPixelRGBA(&image, x, y, (ROSE_RGBA) { rand() % 255, rand() % 255, rand() % 255, 255 });
        //ROSE_SpriteUpdateTexture(&sprite, &image);
        //
        //x++;
        //if (x >= image._width) {
        //    x = 0;
        //    y++;
        //    y %= image._height;
        //}
        
        ROSE_WindowBeginFrame(ROSE_COLOR_BLACK);
        ROSE_MeshDraw(&triangles);
        ROSE_MeshDraw(&lines);
        ROSE_MeshDraw(&points);
        ROSE_ButtonDraw(&button);
        ROSE_SpriteDraw(&sprite);
        ROSE_WindowEndFrame();

        if (ROSE_ButtonPressed(&button))
            fprintf(stdout, "HELLO!\n");
    }

    ROSE_SpriteDestroy(&sprite);
    ROSE_ButtonDestroy(&button);
    ROSE_MeshDestroy(&triangles);
    ROSE_MeshDestroy(&lines);
    ROSE_MeshDestroy(&points);
    ROSE_Quit();
}

#endif /* DEMO_APPLICATION */

