
/*
 *   Source File [main.c]
 */

#include "rose.h"

int main(void) {
	ROSE_Init("App", 1200, 700, TRUE);
	ROSE_WindowToggleVSync(TRUE);

	while (ROSE_WindowPollEvents()) {
		ROSE_WindowClearScreen(ROSE_COLOR_PURPLE);
		ROSE_WindowSwapBuffers();
	}
	
	ROSE_Quit();
	return EXIT_SUCCESS;
}

