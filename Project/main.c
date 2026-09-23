
/*
 *   Source File [main.c]
 */

#include "rose.h"

int main(void) {
	ROSE_Init("App", 1200, 700, TRUE);
	ROSE_WindowToggleVSync(TRUE);

	ROSE_Text* text = ROSE_TextCreate("hello!");

	while (ROSE_WindowIsOpen()) {
		ROSE_WindowClearScreen((ROSE_Color){ 0.1f, 0.1f, 0.1f, 1.0f });
		ROSE_WindowDrawText(text, 20, 20, 5.0, ROSE_COLOR_RED);
		ROSE_WindowRender();
	}
	
	ROSE_TextDestroy(text);
	ROSE_Quit();
	return EXIT_SUCCESS;
}

