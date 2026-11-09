
/*
 *   Source [main.c]
 */

#define __MAIN__
#include "rose.h"

int main(void)
{
	ROSE_Init("My Window", 1600, 900);
	ROSE_WindowVSync(true);



	while (ROSE_WindowIsOpen())
	{
		ROSE_WindowFrameClear(ROSE_COLOR(50, 50, 50, 255));
		ROSE_WindowFrameRender();
	}

	ROSE_Quit();
	return EXIT_SUCCESS;
}

