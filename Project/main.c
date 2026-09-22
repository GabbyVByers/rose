
/*
 *   Source File [main.c]
 */

#include "rose.h"

int main(void) {
	ROSE_Init("App", 1200, 700, TRUE);
	ROSE_ToggleVSync(TRUE);

	while (ROSE_PollEvents()) {
		ROSE_ClearScreen(ROSE_COLOR_PURPLE);
		ROSE_SwapBuffers();
	}
	
	ROSE_Quit();
	return EXIT_SUCCESS;
}

