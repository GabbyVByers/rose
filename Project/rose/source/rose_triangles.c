
/*
 *   Source File [rose_triangles.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

ROSE_Triangles ROSE_TrianglesCreate(ROSE_Vertex2D* vertices, uint32_t num_vertices)
{
	ROSE_Triangles shape = ROSE_TrianglesCreateEmpty();
	ROSE_TrianglesUpdateVertices(&shape, vertices, num_vertices);
	return shape;
}

ROSE_Triangles ROSE_TrianglesCreateEmpty(void)
{
	return (ROSE_Triangles) {
		.scale = 1.0,
		.xpos = 0.0,
		.ypos = 0.0,
		.color = ROSE_COLOR_WHITE,
		._num_vertices = 0,
		._vertex_capacity = 0,
		._vertex_buffer = NULL,
		._transfer_buffer = NULL,
	};
}

void ROSE_TrianglesUpdateVertices(ROSE_Triangles* triangles, ROSE_Vertex2D* vertices, uint32_t num_vertices)
{
	ROSE__TrianglesReserveVertexCapacity(triangles, num_vertices);
	triangles->_num_vertices = num_vertices;

	if (num_vertices == 0)
		return;

	uint32_t vertex_buffer_size = num_vertices * (uint32_t)sizeof(ROSE_Vertex2D);
	void* transfer_buffer_beginning = SDL_MapGPUTransferBuffer(rose__device, triangles->_transfer_buffer, true);
	memcpy(transfer_buffer_beginning, vertices, (size_t)vertex_buffer_size);
	SDL_UnmapGPUTransferBuffer(rose__device, triangles->_transfer_buffer);

	SDL_GPUTransferBufferLocation source_buffer_location = {
		.transfer_buffer = triangles->_transfer_buffer,
	};

	SDL_GPUBufferRegion destination_buffer_region = {
		.buffer = triangles->_vertex_buffer,
		.size = vertex_buffer_size,
	};

	SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(rose__device);
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
	SDL_UploadToGPUBuffer(copy_pass, &source_buffer_location, &destination_buffer_region, true);
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(command_buffer);
}

void ROSE_TrianglesDestroy(ROSE_Triangles* triangles)
{
	if (triangles) {
		if (triangles->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, triangles->_vertex_buffer);
		if (triangles->_transfer_buffer) SDL_ReleaseGPUTransferBuffer(rose__device, triangles->_transfer_buffer);
		*triangles = (ROSE_Triangles) { 0 };
	}
}

void ROSE_TrianglesDraw(ROSE_Triangles* triangles)
{
	if (rose_screen.minimized || (triangles->_num_vertices == 0))
		return;

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = triangles->_vertex_buffer,
	};

	double aspect_ratio = (double)rose_screen.width / (double)rose_screen.height;
	double ww = triangles->scale * (1.0 / aspect_ratio);
	double hh = triangles->scale;
	double xx = triangles->xpos * (1.0 / aspect_ratio);
	double yy = triangles->ypos;

	float w = (float)ww;
	float h = (float)hh;
	float x = (float)xx;
	float y = (float)yy;

	typedef struct Uniform {
		float TransformMatrix[16];
		float TintColor[4];
	} Uniform;

	Uniform uniform = {
		.TransformMatrix = {
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, 1, 0,
			x, y, 0, 1,
		},
		.TintColor = {
			triangles->color.r,
			triangles->color.g,
			triangles->color.b,
			triangles->color.a,
		}
	};

	SDL_BindGPUGraphicsPipeline(rose__render_pass, rose__triangles_pipeline);
	SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(rose__render_pass, 0, &buffer_binding, 1);
	SDL_DrawGPUPrimitives(rose__render_pass, triangles->_num_vertices, 1, 0, 0);
}

void ROSE__TrianglesReserveVertexCapacity(ROSE_Triangles* triangles, uint32_t num_vertices)
{
	if (num_vertices <= triangles->_vertex_capacity)
		return;

	const uint32_t min_capacity = 32;
	uint32_t capacity = triangles->_vertex_capacity;
	if (capacity == 0) {
		capacity = min_capacity;
	}

	while (capacity < num_vertices)
		capacity *= 2;

	uint32_t vertex_buffer_size = capacity * (uint32_t)sizeof(ROSE_Vertex2D);

	if (triangles->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, triangles->_vertex_buffer);
	if (triangles->_transfer_buffer) SDL_ReleaseGPUTransferBuffer(rose__device, triangles->_transfer_buffer);

	SDL_GPUBufferCreateInfo buffer_create_info = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = vertex_buffer_size,
	};

	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = vertex_buffer_size,
	};

	triangles->_vertex_buffer = SDL_CreateGPUBuffer(rose__device, &buffer_create_info);
	triangles->_transfer_buffer = SDL_CreateGPUTransferBuffer(rose__device, &transfer_buffer_create_info);
	triangles->_vertex_capacity = capacity;
}

