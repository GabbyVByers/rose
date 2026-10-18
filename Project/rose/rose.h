
/*
 *   Header File [rose.h]
 */

#ifndef ROSE_ROSE_HEADER_GUARD
#define ROSE_ROSE_HEADER_GUARD

#pragma warning (push, 0)
#include "SDL3/SDL.h"
#include "stb_image.h"
#include "stb_image_write.h"
#pragma warning (pop)

#include "rose_scancodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define ROSE_ASCII_WIDTH 7
#define ROSE_ASCII_HEIGHT 11

typedef struct { float   r, g, b, a; } ROSE_Color;
typedef struct { uint8_t r, g, b, a; } ROSE_RGBA;

typedef struct { float     x, y, z; } ROSE_Vec3f;
typedef struct { double    x, y, z; } ROSE_Vec3d;
typedef struct { int32_t   x, y, z; } ROSE_Vec3i;
typedef struct { uint32_t  x, y, z; } ROSE_Vec3u;
typedef struct { ptrdiff_t x, y, z; } ROSE_Vec3iz;
typedef struct { size_t    x, y, z; } ROSE_Vec3uz;

typedef struct { float  w, x, y, z; } ROSE_Quaternion4f;
typedef struct { double w, x, y, z; } ROSE_Quaternion4d;

typedef struct { float  _[16]; } ROSE_Matrix4x4f;
typedef struct { double _[16]; } ROSE_Matrix4x4d;

typedef struct {
	float position[2];
	float texcoords[2];
} ROSE_VertexSprite;

typedef struct {
	float position[2];
} ROSE_Vertex2D;

typedef struct {
	uint32_t width;
	uint32_t height;
	uint8_t* pixels;
} ROSE_Image;

typedef struct {
	double scale;
	double xpos;
	double ypos;
	ROSE_Color color;
	uint32_t num_characters;
	uint32_t num_vertices;
	SDL_GPUBuffer* vertex_buffer;
} ROSE_Text;

typedef struct {
	double scale;
	double xpos;
	double ypos;
	double width;
	double height;
	ROSE_Color color;
	SDL_GPUBuffer* vertex_buffer;
	SDL_GPUTexture* texture;
} ROSE_Sprite;

typedef struct {
	ROSE_Color color;
	uint32_t num_vertices;
	SDL_GPUBuffer* vertex_buffer;
} ROSE_Shape2D;

void  ROSE_Init(const char*, uint32_t, uint32_t, bool);
void  ROSE_Quit(void);

void  ROSE_WindowToggleVSync(bool);
bool  ROSE_WindowIsOpen(void);
void  ROSE_WindowClear(ROSE_Color);
void  ROSE_WindowRender(void);

bool  ROSE_MousePressing(uint32_t);
bool  ROSE_MousePressed(uint32_t);
bool  ROSE_MouseReleased(uint32_t);
void  ROSE_MouseHide(void);
void  ROSE_MouseReveal(void);

bool  ROSE_KeyboardPressing(ROSE_Scancode);
bool  ROSE_KeyboardPressed(ROSE_Scancode);
bool  ROSE_KeyboardReleased(ROSE_Scancode);

ROSE_Image  ROSE_ImageCreate(uint32_t, uint32_t);
void        ROSE_ImageDestroy(ROSE_Image*);
ROSE_Image  ROSE_ImageLoadFromFile(const char*);
void        ROSE_ImageSavePNG(ROSE_Image*, const char*);
void        ROSE_ImageResize(ROSE_Image*, uint32_t, uint32_t);
ROSE_Color  ROSE_ImageGetPixelColor(ROSE_Image*, uint32_t, uint32_t);
void        ROSE_ImageSetPixelColor(ROSE_Image*, uint32_t, uint32_t, ROSE_Color);

ROSE_Text   ROSE_TextCreate(const char* string);
void        ROSE_TextDestroy(ROSE_Text* text);
void        ROSE_TextDraw(ROSE_Text* text);

ROSE_Sprite ROSE_SpriteCreate(ROSE_Image* image);
void        ROSE_SpriteDestroy(ROSE_Sprite* sprite);
void        ROSE_SpriteDraw(ROSE_Sprite* sprite);

SDL_GPUSampler*          ROSE_SDLCreateSampler(void);
SDL_GPUTexture*          ROSE_SDLCreateDepthTexture(void);
SDL_GPUTexture*          ROSE_SDLCreateTexture(ROSE_Image*);
SDL_GPUBuffer*           ROSE_SDLCreateVertexBuffer(void*, uint32_t, uint32_t);
SDL_GPUGraphicsPipeline* ROSE_SDLCreateGraphicsPipelineSprite(void);

#endif /* ROSE_ROSE_HEADER_GUARD */

