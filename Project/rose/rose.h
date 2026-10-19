
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

#if defined(NDEBUG)
#define ROSE_RELEASE
#else
#define ROSE_DEBUG
#endif

#if defined(__SANITIZE_ADDRESS__)
#pragma message("ASan ENABLED")
#else
#pragma message("ASan DISABLED")
#endif

#if !defined(__SANITIZE_ADDRESS__) && defined(ROSE_DEBUG)
#define ROSE_CRTDBG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

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

#define ROSE_COLOR(r, g, b, a) \
((ROSE_Color){                 \
(float)(r) / 255.0f,           \
(float)(g) / 255.0f,           \
(float)(b) / 255.0f,           \
(float)(a) / 255.0f })

#define ROSE_COLOR_WHITE  ROSE_COLOR(255, 255, 255, 255)
#define ROSE_COLOR_BLACK  ROSE_COLOR(  0,   0,   0, 255)
#define ROSE_COLOR_RED    ROSE_COLOR(255,   0,   0, 255)
#define ROSE_COLOR_GREEN  ROSE_COLOR(  0, 255,   0, 255)
#define ROSE_COLOR_BLUE   ROSE_COLOR(  0,   0, 255, 255)
#define ROSE_COLOR_CYAN   ROSE_COLOR(  0, 255, 255, 255)
#define ROSE_COLOR_PURPLE ROSE_COLOR(255,   0, 255, 255)
#define ROSE_COLOR_YELLOW ROSE_COLOR(255, 255,   0, 255)
#define ROSE_COLOR_TRANS  ROSE_COLOR(255, 255, 255,   0)

typedef struct ROSE_ScreenInfo {
	uint32_t width;  // global (read only)
	uint32_t height; // global (read only)
	bool minimized;  // global (read only)
} ROSE_ScreenInfo;

typedef struct ROSE_MouseInfo {
	float xpos;   // global (read only)
	float ypos;   // global (read only)
	float xvel;   // global (read only)
	float yvel;   // global (read only)
	float scroll; // global (read only)
} ROSE_MouseInfo;

extern ROSE_ScreenInfo rose_screen;
extern ROSE_MouseInfo  rose_mouse;

typedef struct ROSE_VertexSprite {
	float position[2];
	float texcoords[2];
} ROSE_VertexSprite;

typedef struct ROSE_Vertex2D {
	float position[2];
} ROSE_Vertex2D;

typedef struct ROSE_Image {
	uint32_t _width;
	uint32_t _height;
	uint8_t* _pixels;
} ROSE_Image;

typedef struct ROSE_Text {
	double scale;
	double xpos;
	double ypos;
	ROSE_Color color;
	uint32_t _num_characters;
	uint32_t _num_vertices;
	uint32_t _character_capacity;
	SDL_GPUBuffer* _vertex_buffer;
	SDL_GPUTransferBuffer* _transfer_buffer;
} ROSE_Text;

typedef struct ROSE_Sprite {
	double scale;
	double xpos;
	double ypos;
	double _width;
	double _height;
	ROSE_Color color;
	SDL_GPUTexture* _texture;
} ROSE_Sprite;

typedef struct ROSE_Triangles {
	double scale;
	double xpos;
	double ypos;
	ROSE_Color color;
	uint32_t _num_vertices;
	uint32_t _vertex_capacity;
	SDL_GPUBuffer* _vertex_buffer;
	SDL_GPUTransferBuffer* _transfer_buffer;
} ROSE_Triangles;

typedef struct ROSE_Points {
	double scale;
	double xpos;
	double ypos;
	double diameter;
	ROSE_Color color;
	uint32_t _num_vertices;
	uint32_t _vertex_capacity;
	SDL_GPUBuffer* _vertex_buffer;
	SDL_GPUTransferBuffer* _transfer_buffer;
} ROSE_Points;

typedef struct ROSE_Lines {
	double scale;
	double xpos;
	double ypos;
	double thickness;
	ROSE_Color color;
	uint32_t _num_vertices;
	uint32_t _vertex_capacity;
	SDL_GPUBuffer* _vertex_buffer;
	SDL_GPUTransferBuffer* _transfer_buffer;
} ROSE_Lines;

void ROSE_Init(const char* title, uint32_t screen_width, uint32_t screen_height);
void ROSE_Quit(void);

void ROSE_WindowToggleVSync(bool vsync);
bool ROSE_WindowPollEvents(void);
void ROSE_WindowClear(ROSE_Color clear_color);
void ROSE_WindowRender(void);

bool ROSE_MousePressing(uint32_t mouse_button);
bool ROSE_MousePressed(uint32_t mouse_button);
bool ROSE_MouseReleased(uint32_t mouse_button);
void ROSE_MouseHide(void);
void ROSE_MouseReveal(void);

bool ROSE_KeyboardPressing(ROSE_Scancode scancode);
bool ROSE_KeyboardPressed(ROSE_Scancode scancode);
bool ROSE_KeyboardReleased(ROSE_Scancode scancode);

ROSE_Image     ROSE_ImageCreate(uint32_t width, uint32_t height);
void           ROSE_ImageDestroy(ROSE_Image* image);
ROSE_Image     ROSE_ImageLoadFromFile(const char* path);
void           ROSE_ImageSavePNG(ROSE_Image* image, const char* path);
void           ROSE_ImageResize(ROSE_Image* image, uint32_t new_width, uint32_t new_height);
ROSE_Color     ROSE_ImageGetPixelColor(ROSE_Image* image, uint32_t xpos, uint32_t ypos);
ROSE_RGBA      ROSE_ImageGetPixelRGBA(ROSE_Image* image, uint32_t xpos, uint32_t ypos);
void           ROSE_ImageSetPixelColor(ROSE_Image* image, uint32_t xpos, uint32_t ypos, ROSE_Color pixel_color);
void           ROSE_ImageSetPixelRGBA(ROSE_Image* image, uint32_t xpos, uint32_t ypos, ROSE_RGBA pixel_color);

ROSE_Text      ROSE_TextCreate(const char* string);
ROSE_Text      ROSE_TextCreateEmpty(void);
void           ROSE_TextDestroy(ROSE_Text* text);
void           ROSE_TextUpdateString(ROSE_Text* text, const char* new_string);
void           ROSE_TextDraw(ROSE_Text* text);
void           ROSE__TextWriteCharactersToVertices(ROSE_VertexSprite* vertices, const char* string, uint32_t num_characters);
void           ROSE__TextReserveCharacterCapacity(ROSE_Text* text, uint32_t num_characters);

ROSE_Sprite    ROSE_SpriteCreate(ROSE_Image* image);
ROSE_Sprite    ROSE_SpriteCreateEmpty(uint32_t width, uint32_t height);
ROSE_Sprite    ROSE_SpriteLoadFromFile(const char* path);
void           ROSE_SpriteDestroy(ROSE_Sprite* sprite);
void           ROSE_SpriteUploadTexture(ROSE_Sprite* sprite, ROSE_Image* image);
void           ROSE_SpriteDraw(ROSE_Sprite* sprite);

ROSE_Triangles ROSE_TrianglesCreate(ROSE_Vertex2D* vertices, uint32_t num_vertices);
ROSE_Triangles ROSE_TrianglesCreateEmpty(void);
void           ROSE_TrianglesUpdateVertices(ROSE_Triangles* triangles, ROSE_Vertex2D* vertices, uint32_t num_vertices);
void           ROSE_TrianglesDestroy(ROSE_Triangles* triangles);
void           ROSE_TrianglesDraw(ROSE_Triangles* triangles);
void           ROSE__TrianglesReserveVertexCapacity(ROSE_Triangles* triangles, uint32_t num_vertices);

ROSE_Points    ROSE_PointsCreate(ROSE_Vertex2D* vertices, uint32_t num_vertices);
ROSE_Points    ROSE_PointsCreateEmpty(void);
void           ROSE_PointsUpdateVertices(ROSE_Points* points, ROSE_Vertex2D* vertices, uint32_t num_vertices);
void           ROSE_PointsDestroy(ROSE_Points* points);
void           ROSE_PointsDraw(ROSE_Points* points);
void           ROSE__PointsReserveVertexCapacity(ROSE_Points* points, uint32_t num_vertices);

ROSE_Lines     ROSE_LinesCreate(ROSE_Vertex2D* vertices, uint32_t num_vertices);
ROSE_Lines     ROSE_LinesCreateEmpty(void);
void           ROSE_LinesUpdateVertices(ROSE_Lines* lines, ROSE_Vertex2D* vertices, uint32_t num_vertices);
void           ROSE_LinesDestroy(ROSE_Lines* lines);
void           ROSE_LinesDraw(ROSE_Lines* lines);
void           ROSE__LinesReserveVertexCapacity(ROSE_Lines* lines, uint32_t num_vertices);

SDL_GPUSampler*          ROSE__SDLCreateSampler(void);
SDL_GPUTexture*          ROSE__SDLCreateTexture(ROSE_Image* image);
SDL_GPUBuffer*           ROSE__SDLCreateVertexBuffer(void* vertices, uint32_t num_vertices, uint32_t vertex_size);
SDL_GPUShader*           ROSE__SDLLoadShader(const char* path, SDL_GPUShaderStage stage, uint32_t num_uniform_buffers, uint32_t num_samplers);
SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipeline(const char* vertex_path, const char* fragment_path, uint32_t num_fragment_samplers, SDL_GPUVertexInputState vertex_input_state, SDL_GPUPrimitiveType primitive_type);
SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipelineSprite(void);
SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipelineTriangles(void);
SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipelinePoints(void);
SDL_GPUGraphicsPipeline* ROSE__SDLCreateGraphicsPipelineLines(void);

#endif /* ROSE_ROSE_HEADER_GUARD */

