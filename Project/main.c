
/*
 *   Source [main.c]
 */

#define __MAIN__
#include "rose.h"

static float randf(void)
{
	return (float)rand() / (float)RAND_MAX;
}

int main(void)
{
	ROSE_Init("My Window", 1600, 900);
	ROSE_WindowVSync(true);

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
		{ randf() * width, randf() * height },
	};

	uInt32 vertex_count = sizeof(vertices) / sizeof(vertices[0]);

	ROSE_Mesh mesh = ROSE_MeshCreate(vertices, vertex_count);
	mesh.color = ROSE_COLOR_PURPLE;
	mesh.primitive = ROSE_LINES | ROSE_POINTS;
	mesh.diameter = 7;

	ROSE_Slider slider = ROSE_SliderCreate();
	slider.width = 200;
	slider.scale = 3.0f;
	slider.xpos = 50.0f;
	slider.ypos = 50.0f;

	while (ROSE_WindowIsOpen())
	{
		ROSE_WindowFrameClear(ROSE_COLOR_BLACK);
		ROSE_MeshDraw(&mesh);
		ROSE_SliderDraw(&slider);
		ROSE_WindowFrameRender();
	}

	ROSE_MeshDestroy(&mesh);

	ROSE_Quit();
	return EXIT_SUCCESS;
}

