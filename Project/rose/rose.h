
/*
 *   Header File [rose.h]
 */

#ifndef ROSE_ROSE_HEADER_GUARD
#define ROSE_ROSE_HEADER_GUARD

#include "SDL3/SDL.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "rose_scancodes.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t usize;
typedef ptrdiff_t isize;

typedef struct { u8  r, g, b, a; } ROSE_RGBA;
typedef struct { f32 r, g, b, a; } ROSE_Color4;

#define ROSE_COLOR(r, g, b, a) \
((ROSE_Color4) {               \
(float)r / 255.0f,             \
(float)g / 255.0f,             \
(float)b / 255.0f,             \
(float)a / 255.0f })

#define ROSE_COLOR_WHITE  ROSE_COLOR(255, 255, 255, 255)
#define ROSE_COLOR_BLACK  ROSE_COLOR(  0,   0,   0, 255)
#define ROSE_COLOR_RED    ROSE_COLOR(255,   0,   0, 255)
#define ROSE_COLOR_GREEN  ROSE_COLOR(  0, 255,   0, 255)
#define ROSE_COLOR_BLUE   ROSE_COLOR(  0,   0, 255, 255)
#define ROSE_COLOR_CYAN   ROSE_COLOR(  0, 255, 255, 255)
#define ROSE_COLOR_PURPLE ROSE_COLOR(255,   0, 255, 255)
#define ROSE_COLOR_YELLOW ROSE_COLOR(255, 255,   0, 255)
#define ROSE_COLOR_TRANS  ROSE_COLOR(255, 255, 255,   0)

typedef struct {
	f32 position[2];
	f32 texcoords[2];
} ROSE_Vertex2D;

typedef struct {
	f32 position[3];
	f32 color[4];
	f32 texcoords[2];
} ROSE_Vertex3D;

typedef struct ROSE_Image  ROSE_Image;
typedef struct ROSE_Text   ROSE_Text;
typedef struct ROSE_Sprite ROSE_Sprite;
typedef struct ROSE_Mesh   ROSE_Mesh;

void ROSE_Init(const char*, u32, u32, bool);
void ROSE_Quit(void);

void ROSE_WindowToggleVSync(bool);
bool ROSE_WindowIsOpen(void);
void ROSE_WindowClear(ROSE_Color4);
void ROSE_WindowRender(void);
u32  ROSE_WindowWidth(void);
u32  ROSE_WindowHeight(void);

bool ROSE_MousePressing(u32);
bool ROSE_MousePressed(u32);
bool ROSE_MouseReleased(u32);
void ROSE_MouseHide(void);
void ROSE_MouseReveal(void);
f32  ROSE_MouseScroll(void);
f32  ROSE_MousePositionX(void);
f32  ROSE_MousePositionY(void);
f32  ROSE_MouseVelocityX(void);
f32  ROSE_MouseVelocityY(void);

bool ROSE_KeyboardPressing(ROSE_Scancode);
bool ROSE_KeyboardPressed(ROSE_Scancode);
bool ROSE_KeyboardReleased(ROSE_Scancode);

ROSE_Image*     ROSE_ImageCreate(u32, u32);
void            ROSE_ImageDestroy(ROSE_Image*);
ROSE_Image*     ROSE_ImageLoad(const char*);
void            ROSE_ImageSavePNG(ROSE_Image*, const char*);
void            ROSE_ImageResize(ROSE_Image*, u32, u32);
ROSE_RGBA       ROSE_ImageGetPixelRGBA(ROSE_Image*, u32, u32);
ROSE_Color4     ROSE_ImageGetPixelColor4(ROSE_Image*, u32, u32);
void            ROSE_ImageSetPixelRGBA(ROSE_Image*, ROSE_RGBA, u32, u32);
void            ROSE_ImageSetPixelColor4(ROSE_Image*, ROSE_Color4, u32, u32);
u32             ROSE_ImageWidth(ROSE_Image*);
u32             ROSE_ImageHeight(ROSE_Image*);
const u8*       ROSE_ImagePixels(ROSE_Image*);

ROSE_Text*      ROSE_TextCreate(const char*);
void            ROSE_TextDestroy(ROSE_Text*);
void            ROSE_TextUpdateString(ROSE_Text*, const char*);
void            ROSE_TextDraw(ROSE_Text*, i32, i32, float, ROSE_Color4);
u32             ROSE_TextWidth(ROSE_Text*);
u32             ROSE_TextHeight(ROSE_Text*);

ROSE_Sprite*    ROSE_SpriteCreate(ROSE_Image*);
void            ROSE_SpriteDestroy(ROSE_Sprite*);
void            ROSE_SpriteDraw(ROSE_Sprite*, i32, i32, float, ROSE_Color4);
u32             ROSE_SpriteWidth(ROSE_Sprite*);
u32             ROSE_SpriteHeight(ROSE_Sprite*);

SDL_GPUTexture* ROSE_SDLCreateTexture(ROSE_Image*);
SDL_GPUBuffer*  ROSE_SDLCreateVertexBuffer(ROSE_Vertex2D*, u32, u32);

#endif /* ROSE_ROSE_HEADER_GUARD */

