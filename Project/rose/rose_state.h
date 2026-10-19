
/*
 *   Header File [rose_state.h]
 */

#ifndef ROSE_STATE_HEADER_GUARD
#define ROSE_STATE_HEADER_GUARD

#include "rose.h"

#define ROSE_GLOBAL(type, var) \
type var = { 0 };

#ifdef ROSE_EXTERN
#undef ROSE_GLOBAL
#define ROSE_GLOBAL(type, var) \
extern type var;
#endif

ROSE_GLOBAL(ROSE_ScreenInfo, rose_screen)
ROSE_GLOBAL(ROSE_MouseInfo, rose_mouse)

ROSE_GLOBAL(SDL_Window*, rose__window)
ROSE_GLOBAL(SDL_GPUDevice*, rose__device)
ROSE_GLOBAL(SDL_GPUSampler*, rose__sampler)
ROSE_GLOBAL(SDL_GPUTexture*, rose__ascii_texture)
ROSE_GLOBAL(SDL_GPUTexture*, rose__dummy_texture)
ROSE_GLOBAL(SDL_GPUGraphicsPipeline*, rose__points_pipeline)
ROSE_GLOBAL(SDL_GPUGraphicsPipeline*, rose__sprite_pipeline)
ROSE_GLOBAL(SDL_GPUGraphicsPipeline*, rose__triangles_pipeline)
ROSE_GLOBAL(SDL_GPUGraphicsPipeline*, rose__lines_pipeline)
ROSE_GLOBAL(SDL_GPUBuffer*, rose__sprite_quad_buffer)
ROSE_GLOBAL(SDL_GPURenderPass*, rose__render_pass)
ROSE_GLOBAL(SDL_GPUTexture*, rose__swapchain_texture)
ROSE_GLOBAL(SDL_GPUCommandBuffer*, rose__command_buffer)

ROSE_GLOBAL(float, rose__saved_mouse_px)
ROSE_GLOBAL(float, rose__saved_mouse_py)
ROSE_GLOBAL(uint32_t, rose__curr_mouse_state)
ROSE_GLOBAL(uint32_t, rose__prev_mouse_state)
ROSE_GLOBAL(bool, rose__curr_keyboard_state[SDL_SCANCODE_COUNT])
ROSE_GLOBAL(bool, rose__prev_keyboard_state[SDL_SCANCODE_COUNT])

#endif /* ROSE_STATE_HEADER_GUARD */

