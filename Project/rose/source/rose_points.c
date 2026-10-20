
/*
 *   Source File [rose_points.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

ROSE_Points ROSE_PointsCreate(ROSE_Vertex2D* vertices, uint32_t num_vertices)
{
	ROSE_Points points = ROSE_PointsCreateEmpty();
	ROSE_PointsUpdateVertices(&points, vertices, num_vertices);
	return points;
}

ROSE_Points ROSE_PointsCreateEmpty(void)
{
	return (ROSE_Points) {
		.scale = 1.0f,
		.xpos = 0.0f,
		.ypos = 0.0f,
		.diameter = 1,
		.color = ROSE_COLOR_WHITE,
		._num_vertices = 0,
		._vertex_capacity = 0,
		._vertex_buffer = NULL,
		._transfer_buffer = NULL,
	};
}

void ROSE_PointsUpdateVertices(ROSE_Points* points, ROSE_Vertex2D* vertices, uint32_t num_vertices)
{
	ROSE__PointsReserveVertexCapacity(points, num_vertices);
	points->_num_vertices = num_vertices;

	if (num_vertices == 0)
		return;

	uint32_t vertex_buffer_size = num_vertices * (uint32_t)sizeof(ROSE_Vertex2D);
	void* transfer_buffer_beginning = SDL_MapGPUTransferBuffer(rose__device, points->_transfer_buffer, true);
	memcpy(transfer_buffer_beginning, vertices, (size_t)vertex_buffer_size);
	SDL_UnmapGPUTransferBuffer(rose__device, points->_transfer_buffer);

	SDL_GPUTransferBufferLocation source_buffer_location = {
		.transfer_buffer = points->_transfer_buffer,
	};

	SDL_GPUBufferRegion destination_buffer_region = {
		.buffer = points->_vertex_buffer,
		.size = vertex_buffer_size,
	};

	SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(rose__device);
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
	SDL_UploadToGPUBuffer(copy_pass, &source_buffer_location, &destination_buffer_region, true);
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(command_buffer);
}

void ROSE_PointsDestroy(ROSE_Points* points)
{
	if (points) {
		if (points->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, points->_vertex_buffer);
		if (points->_transfer_buffer) SDL_ReleaseGPUTransferBuffer(rose__device, points->_transfer_buffer);
		*points = (ROSE_Points){ 0 };
	}
}

void ROSE_PointsDraw(ROSE_Points* points)
{
	if (rose_screen.minimized || (points->_num_vertices == 0))
		return;

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = points->_vertex_buffer,
	};

	float ratio = (float)rose_screen.height / (float)rose_screen.width;
	float w = points->scale * ratio;
	float h = points->scale;
	float x = points->xpos * ratio;
	float y = points->ypos;

	typedef struct Uniform {
		float TransformMatrix[16];
		float TintColor[4];
		float PixelDiameter;
	} Uniform;

	Uniform uniform = {
		.TransformMatrix = {
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, 1, 0,
			x, y, 0, 1,
		},
		.TintColor = {
			points->color.r,
			points->color.g,
			points->color.b,
			points->color.a,
		},
		.PixelDiameter = (float)points->diameter,
	};

	SDL_BindGPUGraphicsPipeline(rose__render_pass, rose__points_pipeline);
	SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(rose__render_pass, 0, &buffer_binding, 1);
	SDL_DrawGPUPrimitives(rose__render_pass, points->_num_vertices, 1, 0, 0);
}

void ROSE__PointsReserveVertexCapacity(ROSE_Points* points, uint32_t num_vertices)
{
	if (num_vertices <= points->_vertex_capacity)
		return;

	const uint32_t min_capacity = 32;
	uint32_t capacity = points->_vertex_capacity;
	if (capacity == 0) {
		capacity = min_capacity;
	}

	while (capacity < num_vertices)
		capacity *= 2;

	uint32_t vertex_buffer_size = capacity * (uint32_t)sizeof(ROSE_Vertex2D);

	if (points->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, points->_vertex_buffer);
	if (points->_transfer_buffer) SDL_ReleaseGPUTransferBuffer(rose__device, points->_transfer_buffer);

	SDL_GPUBufferCreateInfo buffer_create_info = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = vertex_buffer_size,
	};

	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = vertex_buffer_size,
	};

	points->_vertex_buffer = SDL_CreateGPUBuffer(rose__device, &buffer_create_info);
	points->_transfer_buffer = SDL_CreateGPUTransferBuffer(rose__device, &transfer_buffer_create_info);
	points->_vertex_capacity = capacity;
}

