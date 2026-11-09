
/*
 *   Source [main.c]
 */

#define __MAIN__
#include "rose.h"

#define SPRITE_WIDTH 200
#define SPRITE_HEIGHT 200
#define SPRITE_PADDING 20

static float randf(void)
{
	// [0.0, 1.0]
	return (float)rand() / (float)RAND_MAX;
}

int main(void)
{
	ROSE_Init("My Window", 1600, 900);
	ROSE_WindowVSync(true);

	ROSE_Perlin perlin = ROSE_PerlinCreateEmpty(SPRITE_WIDTH, SPRITE_HEIGHT);
	perlin.layers = 8;
	ROSE_PerlinGenerate(&perlin);

	ROSE_Image image = ROSE_ImageCreateFromPerlin(&perlin);
	ROSE_Sprite sprite = ROSE_SpriteCreate(&image);
	sprite.xpos = SPRITE_PADDING;
	sprite.ypos = SPRITE_PADDING;
	sprite.scale = 2;

	float width = (float)rose_screen.width;
	float height = (float)rose_screen.height;

	ROSE_Vertex2D vertices[] = {
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
		{ randf() * width, randf() * height },
	};

	uInt32 vertex_count = sizeof(vertices) / sizeof(vertices[0]);

	ROSE_Mesh mesh = ROSE_MeshCreate(vertices, vertex_count);
	mesh.color = ROSE_COLOR_PURPLE;
	mesh.primitive = ROSE_LINES | ROSE_POINTS;
	mesh.diameter = 7;

	while (ROSE_WindowIsOpen())
	{
		ROSE_WindowFrameClear(ROSE_COLOR_BLACK);
		ROSE_SpriteDraw(&sprite);
		ROSE_MeshDraw(&mesh);
		ROSE_WindowFrameRender();
	}

	ROSE_SpriteDestroy(&sprite);
	ROSE_ImageDestroy(&image);
	ROSE_PerlinDestroy(&perlin);
	
	ROSE_Quit();
	return EXIT_SUCCESS;
}

