
/*
 *   Source [main.c]
 */

#define __MAIN__
#include "rose.h"

int main(void)
{
	ROSE_Init("Van Gogh's Epic Odyssey", 1600, 900);
	ROSE_WindowVSync(true);

	while (ROSE_WindowPollEvents())
	{
		ROSE_WindowBeginFrame(ROSE_COLOR_BLACK);
		ROSE_WindowEndFrame();
	}

	ROSE_Quit();
	return EXIT_SUCCESS;
}

