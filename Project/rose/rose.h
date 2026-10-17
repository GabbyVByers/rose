
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

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

typedef size_t    usize;
typedef ptrdiff_t isize;

typedef struct {
	f32 position[2];
	f32 texcoords[2];
} ROSE_Vertex2D;

typedef struct {
	f32 position[2];
	f32 color[4];
	f32 texcoords[3];
} ROSE_Vertex3D;

typedef struct {
	u32 _width;
	u32 _height;
	u8* _pixels;
} ROSE_Image;

typedef struct {
	f64 scale;
	f64 xpos;
	f64 ypos;
	f32 color[4];
	u32 _width;
	u32 _height;
	u32 _count;
	SDL_GPUBuffer* _vertices;
} ROSE_Text;

typedef struct {
	f64 scale;
	f64 xpos;
	f64 ypos;
	f32 color[4];
	u32 _width;
	u32 _height;
	SDL_GPUBuffer* _vertices;
	SDL_GPUTexture* _texture;
} ROSE_Sprite;

typedef struct {
	f64 fov;
	f64 near;
	f64 xpos;
	f64 ypos;
	f64 zpos;
	f64 _quat[4];
} ROSE_Camera;

typedef struct {
	f64 scale;
	f64 xpos;
	f64 ypos;
	f64 zpos;
	f64 color[4];
	f64 _quat[4];
	u32 _count;
	SDL_GPUBuffer* _vertices;
	SDL_GPUTexture* _texture;
} ROSE_Mesh;

void ROSE_Init(const char* title, u32 width, u32 height, bool vkdebug);
void ROSE_Quit(void);

void ROSE_WindowToggleVSync(bool vsync);
bool ROSE_WindowIsOpen(void);
void ROSE_WindowClear(float color[4]);
void ROSE_WindowRender(void);

bool ROSE_MousePressing(u32);
bool ROSE_MousePressed(u32);
bool ROSE_MouseReleased(u32);
void ROSE_MouseHide(void);
void ROSE_MouseReveal(void);

bool ROSE_KeyboardPressing(ROSE_Scancode);
bool ROSE_KeyboardPressed(ROSE_Scancode);
bool ROSE_KeyboardReleased(ROSE_Scancode);

ROSE_Image  ROSE_ImageCreate(u32 width, u32 height);
void        ROSE_ImageDestroy(ROSE_Image* image);
void        ROSE_ImageLoadFromFile(ROSE_Image* image, const char* path);
void        ROSE_ImageSavePNG(ROSE_Image* image, const char* path);
void        ROSE_ImageResize(ROSE_Image* image, u32 width, u32 height);
void        ROSE_ImageGetPixelColor(ROSE_Image* image, u32 xpos, u32 ypos, float color[4]);
void        ROSE_ImageSetPixelColor(ROSE_Image* image, u32 xpos, u32 ypos, float color[4]);

ROSE_Text   ROSE_TextCreate(const char* string);
void        ROSE_TextDestroy(ROSE_Text* text);
void        ROSE_TextUpdateString(ROSE_Text* text, const char* string);
void        ROSE_TextDraw(ROSE_Text* text);

ROSE_Sprite ROSE_SpriteCreate(ROSE_Image* image);
void        ROSE_SpriteDestroy(ROSE_Sprite* sprite);
void        ROSE_SpriteDraw(ROSE_Sprite* sprite);

ROSE_Camera ROSE_CameraInit(void);
void        ROSE_CameraRotate(ROSE_Camera*, f32, f32, f32, f32);
void        ROSE_CameraDirectionUp(ROSE_Camera*, f32*, f32*, f32*);
void        ROSE_CameraDirectionRight(ROSE_Camera*, f32*, f32*, f32*);
void        ROSE_CameraDirectionForward(ROSE_Camera*, f32*, f32*, f32*);

ROSE_Mesh   ROSE_MeshCreate(void);
void        ROSE_MeshDestroy(ROSE_Mesh*);
void        ROSE_MeshUploadVertices(ROSE_Mesh*, ROSE_Vertex3D*, u32);
void        ROSE_MeshUploadTexture(ROSE_Mesh*, ROSE_Image*);
void        ROSE_MeshDraw(ROSE_Mesh*, ROSE_Camera*);

SDL_GPUSampler*          ROSE_SDLCreateSampler(void);
SDL_GPUTexture*          ROSE_SDLCreateDepthTexture(void);
SDL_GPUTexture*          ROSE_SDLCreateTexture(ROSE_Image*);
SDL_GPUBuffer*           ROSE_SDLCreateVertexBuffer(ROSE_Vertex2D*, u32, u32);
SDL_GPUGraphicsPipeline* ROSE_SDLCreateGraphicsPipelineSprite(void);
SDL_GPUGraphicsPipeline* ROSE_SDLCreateGraphicsPipelineMesh(void);

#endif /* ROSE_ROSE_HEADER_GUARD */

