
/*
 *   Header File [rose.h]
 */

#ifndef ROSE_ROSE_HEADER_GUARD
#define ROSE_ROSE_HEADER_GUARD

#include "core.h"

/* Public API */

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
ROSE_Color  ROSE_ImageGetPixel(ROSE_Image*, i32, i32);
void        ROSE_ImageSetPixel(ROSE_Image*, i32, i32, ROSE_Color);

ROSE_Sprite* ROSE_SpriteCreate(const char*);
void         ROSE_SpriteDestroy(ROSE_Sprite*);
void         ROSE_SpriteUploadTexture(ROSE_Sprite*);

ROSE_Text* ROSE_TextCreate(const char*);
void       ROSE_TextDestroy(ROSE_Text*);

/* Internal */

SDL_GPUTexture* ROSE_INTERNAL_CreateDepthTexture(void);
SDL_GPUTexture* ROSE_INTERNAL_CreateRenderTexture(i32, i32);
void            ROSE_INTERNAL_UploadImageToRenderTexture(ROSE_Image*, SDL_GPUTexture*);
SDL_GPUBuffer*  ROSE_INTERNAL_CreateVertexBuffer(ROSE_Vertex*, usize);

#endif /* ROSE_ROSE_HEADER_GUARD */

