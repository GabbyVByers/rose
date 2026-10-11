
/*
 *   Header File [rose_state.h]
 */

#ifndef ROSE_STATE_HEADER_GUARD
#define ROSE_STATE_HEADER_GUARD

#include "rose.h"

#define ROSE_GLOBAL(type, var) \
type var = { (type)0 };

#ifdef ROSE_EXTERN
#undef ROSE_GLOBAL
#define ROSE_GLOBAL(type, var) \
extern type var;
#endif

ROSE_GLOBAL(u32, screen_width)
ROSE_GLOBAL(u32, screen_height)
ROSE_GLOBAL(SDL_Window*, window)
ROSE_GLOBAL(SDL_GPUDevice*, device)
ROSE_GLOBAL(SDL_GPUSampler*, sampler)
ROSE_GLOBAL(SDL_GPUTexture*, depth_texture)
ROSE_GLOBAL(SDL_GPUTexture*, ascii_texture)
ROSE_GLOBAL(SDL_GPUGraphicsPipeline*, sprite_pipeline)
ROSE_GLOBAL(SDL_GPUGraphicsPipeline*, mesh_pipeline)

ROSE_GLOBAL(bool, minimized)
ROSE_GLOBAL(SDL_GPURenderPass*, render_pass)
ROSE_GLOBAL(SDL_GPUTexture*, swapchain_texture)
ROSE_GLOBAL(SDL_GPUCommandBuffer*, command_buffer)

ROSE_GLOBAL(float, mouse_px)
ROSE_GLOBAL(float, mouse_py)
ROSE_GLOBAL(float, mouse_vx)
ROSE_GLOBAL(float, mouse_vy)
ROSE_GLOBAL(float, saved_mouse_px)
ROSE_GLOBAL(float, saved_mouse_py)
ROSE_GLOBAL(u32, curr_mouse_state)
ROSE_GLOBAL(u32, prev_mouse_state)
ROSE_GLOBAL(float, mouse_scroll)

ROSE_GLOBAL(bool, curr_keyboard_state[SDL_SCANCODE_COUNT])
ROSE_GLOBAL(bool, prev_keyboard_state[SDL_SCANCODE_COUNT])

#endif /* ROSE_STATE_HEADER_GUARD */

