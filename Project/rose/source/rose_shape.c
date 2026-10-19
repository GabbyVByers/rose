
/*
 *   Source File [rose_gpu.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

ROSE_Shape ROSE_ShapeCreate(ROSE_Vertex2D* vertices, uint32_t num_vertices)
{

}

ROSE_Shape ROSE_ShapeCreateEmpty(void)
{
	return (ROSE_Shape) {
		.color = ROSE_COLOR_WHITE,
		._num_vertices = 0,
		._vertex_buffer = NULL,
	};
}

void ROSE_ShapeUpdateVertices(ROSE_Shape* shape, ROSE_Vertex2D* vertices, uint32_t num_vertices)
{

}

void ROSE_ShapeDestroy(ROSE_Shape* shape)
{

}

void ROSE_ShapeDrawTriangles(ROSE_Shape* shape)
{

}

void ROSE_ShapeDrawLines(ROSE_Shape* shape)
{

}

void ROSE_ShapeDrawPoints(ROSE_Shape* shape, double radius)
{

}

