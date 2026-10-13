
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

void ROSE_Init(const char*, u32, u32, bool);
void ROSE_Quit(void);

void ROSE_WindowToggleVSync(bool);
bool ROSE_WindowIsOpen(void);
void ROSE_WindowClear(ROSE_Color4);
void ROSE_WindowRender(void);

bool ROSE_MousePressing(u32);
bool ROSE_MousePressed(u32);
bool ROSE_MouseReleased(u32);
void ROSE_MouseHide(void);
void ROSE_MouseReveal(void);

bool ROSE_KeyboardPressing(ROSE_Scancode);
bool ROSE_KeyboardPressed(ROSE_Scancode);
bool ROSE_KeyboardReleased(ROSE_Scancode);

/*
 * double > float
 * float or double > i32 (for px/py)
 */

typedef struct {
	u32 width;
	u32 height;
	u8* pixels;
} ROSE_Image;

typedef struct {
	u32 width;
	u32 height;
	u32 full_width;
	u32 num_vertices;
	SDL_GPUBuffer* buffer;
} ROSE_Text;

typedef struct {
	i32 px, py;
	f32 scale;
	ROSE_Color4 color;
	u32 width, height;
	SDL_GPUBuffer* buffer;
	SDL_GPUTexture* texture;
} ROSE_Sprite;

typedef struct {
	f32 fov;
	f32 near_plane;
	f32 px, py, pz;
	f32 qw, qx, qy, qz;
} ROSE_Camera;

typedef struct {
	f32 scale;
	f32 px, py, pz;
	f32 qw, qx, qy, qz;
	ROSE_Color4 color;
	u32 count;
	SDL_GPUBuffer* buffer;
	SDL_GPUTexture* texture;
} ROSE_Mesh;

ROSE_Image  ROSE_ImageCreate(u32, u32);
ROSE_Image  ROSE_ImageLoad(const char*);
void        ROSE_ImageDestroy(ROSE_Image*);
void        ROSE_ImageSavePNG(ROSE_Image*, const char*);
void        ROSE_ImageResize(ROSE_Image*, u32, u32);
ROSE_RGBA   ROSE_ImageGetPixelRGBA(ROSE_Image*, u32, u32);
ROSE_Color4 ROSE_ImageGetPixelColor4(ROSE_Image*, u32, u32);
void        ROSE_ImageSetPixelRGBA(ROSE_Image*, ROSE_RGBA, u32, u32);
void        ROSE_ImageSetPixelColor4(ROSE_Image*, ROSE_Color4, u32, u32);

ROSE_Text*   ROSE_TextCreate(const char*);
void         ROSE_TextDestroy(ROSE_Text*);
void         ROSE_TextUpdateString(ROSE_Text*, const char*);
void         ROSE_TextDraw(ROSE_Text*, i32, i32, f32, ROSE_Color4);
u32          ROSE_TextWidth(ROSE_Text*);
u32          ROSE_TextHeight(ROSE_Text*);

ROSE_Sprite ROSE_SpriteCreate(ROSE_Image*);
void        ROSE_SpriteDestroy(ROSE_Sprite*);
void        ROSE_SpriteDraw(ROSE_Sprite*);

ROSE_Camera ROSE_CameraInit(void);
void        ROSE_CameraRotate(f32, f32, f32, f32);
void        ROSE_CameraDirectionUp(f32*, f32*, f32*);
void        ROSE_CameraDirectionRight(f32*, f32*, f32*);
void        ROSE_CameraDirectionForward(f32*, f32*, f32*);

ROSE_Mesh ROSE_MeshCreate(void);
void      ROSE_MeshDestroy(ROSE_Mesh*);
void      ROSE_MeshUploadVertices(ROSE_Mesh*, ROSE_Vertex3D*, u32);
void      ROSE_MeshUploadTexture(ROSE_Mesh*, ROSE_Image*);
void      ROSE_MeshDraw(ROSE_Mesh*, f32, f32, f32, f32, ROSE_Color4);

SDL_GPUSampler*          ROSE_SDLCreateSampler(void);
SDL_GPUTexture*          ROSE_SDLCreateDepthTexture(void);
SDL_GPUTexture*          ROSE_SDLCreateTexture(ROSE_Image*);
SDL_GPUBuffer*           ROSE_SDLCreateVertexBuffer(ROSE_Vertex2D*, u32, u32);
SDL_GPUGraphicsPipeline* ROSE_SDLCreateGraphicsPipelineSprite(void);
SDL_GPUGraphicsPipeline* ROSE_SDLCreateGraphicsPipelineMesh(void);

#endif /* ROSE_ROSE_HEADER_GUARD */

