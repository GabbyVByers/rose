
/*
 *   Source File [main.c]
 */

#include "rose.h"

int main(void) {
	ROSE_Init("App", 1200, 700, TRUE);
	ROSE_WindowToggleVSync(TRUE);

	while (ROSE_WindowIsOpen()) {
		ROSE_WindowClearScreen(ROSE_COLOR_PURPLE);
		ROSE_WindowRender();
	}
	
	ROSE_Quit();
	return EXIT_SUCCESS;
}

