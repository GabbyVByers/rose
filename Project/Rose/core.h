
/*
 *   Header File [core.h]
 */

#ifndef ROSE_CORE_HEADER_GUARD
#define ROSE_CORE_HEADER_GUARD

#include "SDL3/SDL.h"
#include "stb_image.h"
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ROSE_COLOR_WHITE  ((ROSE_Color){ 1.0f, 1.0f, 1.0f, 1.0f })
#define ROSE_COLOR_BLACK  ((ROSE_Color){ 0.0f, 0.0f, 0.0f, 1.0f })
#define ROSE_COLOR_RED    ((ROSE_Color){ 1.0f, 0.0f, 0.0f, 1.0f })
#define ROSE_COLOR_GREEN  ((ROSE_Color){ 0.0f, 1.0f, 0.0f, 1.0f })
#define ROSE_COLOR_BLUE   ((ROSE_Color){ 0.0f, 0.0f, 1.0f, 1.0f })
#define ROSE_COLOR_PURPLE ((ROSE_Color){ 1.0f, 0.0f, 1.0f, 1.0f })
#define ROSE_COLOR_YELLOW ((ROSE_Color){ 1.0f, 1.0f, 0.0f, 1.0f })
#define ROSE_COLOR_CYAN   ((ROSE_Color){ 0.0f, 1.0f, 1.0f, 1.0f })

#ifdef true
#undef true
#endif /* true */

#ifdef false
#undef false
#endif /* false */

#ifdef bool
#undef bool
#endif /* bool */

typedef _Bool bool;
#define TRUE  ((_Bool)0)
#define FALSE ((_Bool)0)

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

#endif /* ROSE_CORE_HEADER_GUARD */

