
/*
 *   Header File [rose.h]
 */

#ifndef ROSE_ROSE_HEADER_GUARD
#define ROSE_ROSE_HEADER_GUARD

#include "core.h"

typedef struct {
	float pos[2];
	float color[4];
	float uv[2];
} ROSE_Vertex;

typedef struct {
	float r;
	float g;
	float b;
	float a;
} ROSE_Color;

typedef struct ROSE_Image ROSE_Image;
typedef struct ROSE_Sprite ROSE_Sprite;
typedef struct ROSE_Text ROSE_Text;

void ROSE_Init(const char*, i32, i32, bool);
void ROSE_Quit(void);

void ROSE_WindowToggleVSync(bool);
bool ROSE_WindowIsOpen(void);
void ROSE_WindowClearScreen(ROSE_Color);
void ROSE_WindowDrawSprite(ROSE_Sprite*, i32, i32, double, ROSE_Color);
void ROSE_WindowDrawText(ROSE_Text*, i32, i32, double, ROSE_Color);
void ROSE_WindowRender(void);

ROSE_Image* ROSE_ImageCreate(i32, i32);
void        ROSE_ImageDestroy(ROSE_Image*);
ROSE_Image* ROSE_ImageLoadFromFile(const char*);
void        ROSE_ImageSaveAsPNG(ROSE_Image*, const char*);
void        ROSE_ImageResize(ROSE_Image*, i32, i32);
void        ROSE_ImageGetDimensions(ROSE_Image*, i32*, i32*);
ROSE_Color  ROSE_ImageGetPixel(ROSE_Image*, i32, i32);
void        ROSE_ImageSetPixel(ROSE_Image*, i32, i32, ROSE_Color);



SDL_GPUTexture* ROSE_INTERNAL_CreateDepthTexture(void);

#endif /* ROSE_ROSE_HEADER_GUARD */

