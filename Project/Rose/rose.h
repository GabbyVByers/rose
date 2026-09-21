
/*
 *   Header File [rose.h]
 */

#ifndef ROSE_ROSE_HEADER_GUARD
#define ROSE_ROSE_HEADER_GUARD

#include "SDL3/SDL.h"
#include "stb_image.h"
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t    usize;
typedef ptrdiff_t isize;

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

typedef struct ROSE_Sprite ROSE_Sprite;
typedef struct ROSE_Text ROSE_Text;

typedef struct ROSE_Window ROSE_Window;
ROSE_Window* ROSE_WindowCreate(const char*, i32, i32, bool);
void         ROSE_WindowDestroy(ROSE_Window*);
void         ROSE_WindowGetDimensions(i32*, i32*);
void         ROSE_WindowVSync(bool);
bool         ROSE_WindowPollEvents(void);
void         ROSE_WindowClear(ROSE_Color);
void         ROSE_WindowDrawSprite(ROSE_Sprite*, i32, i32, double, ROSE_Color);
void         ROSE_WindowDrawText(ROSE_Text*, i32, i32, double, ROSE_Color);
void         ROSE_WindowRender(void);

typedef struct ROSE_Image ROSE_Image;
ROSE_Image*  ROSE_ImageCreate(i32, i32);
void         ROSE_ImageDestroy(ROSE_Image*);
ROSE_Image*  ROSE_ImageLoadFromFile(const char*);
void         ROSE_ImageSaveAsPNG(ROSE_Image*, const char*);
void         ROSE_ImageResize(ROSE_Image*, i32, i32);
void         ROSE_ImageGetDimensions(ROSE_Image*, i32*, i32*);
ROSE_Color   ROSE_ImageGetPixel(ROSE_Image*, i32, i32);
void         ROSE_ImageSetPixel(ROSE_Image*, i32, i32, ROSE_Color);

#endif /* ROSE_ROSE_HEADER_GUARD */

