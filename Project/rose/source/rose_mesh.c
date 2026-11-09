
/*
 *   Source [rose_mesh.c]
 */

#include "rose.h"

ROSE_Mesh ROSE_MeshCreate(ROSE_Vertex2D* vertices, uInt32 vertex_count)
{
	ROSE_Mesh mesh = ROSE_MeshCreateEmpty();
	ROSE_MeshUpdateVertices(&mesh, vertices, vertex_count);
	return mesh;
}

ROSE_Mesh ROSE_MeshCreateEmpty(void)
{
	return (ROSE_Mesh) {
		.primitive = ROSE_TRIANGLES,
		.scale = 1.0f,
		.xpos = 0.0f,
		.ypos = 0.0f,
		.diameter = 1.0f,
		.color = ROSE_COLOR_WHITE,
		._vertex_count = 0,
		._vertex_capacity = 0,
		._vertex_buffer = NULL,
		._transfer_buffer = NULL,
	};
}

void ROSE_MeshUpdateVertices(ROSE_Mesh* mesh, ROSE_Vertex2D* vertices, uInt32 vertex_count)
{
	ROSE__MeshReserveVertexCapacity(mesh, vertex_count);
	mesh->_vertex_count = vertex_count;

	if (vertex_count == 0) {
		return;
	}

	uInt32 vertex_buffer_size = vertex_count * (uInt32)sizeof(ROSE_Vertex2D);
	void* transfer_buffer_beginning = SDL_MapGPUTransferBuffer(rose__device, mesh->_transfer_buffer, true);
	memcpy(transfer_buffer_beginning, vertices, vertex_buffer_size);
	SDL_UnmapGPUTransferBuffer(rose__device, mesh->_transfer_buffer);

	SDL_GPUTransferBufferLocation source_buffer_location = {
		.transfer_buffer = mesh->_transfer_buffer,
	};

	SDL_GPUBufferRegion destination_buffer_region = {
		.buffer = mesh->_vertex_buffer,
		.size = vertex_buffer_size,
	};

	SDL_UploadToGPUBuffer(ROSE__SDLAcquireCopyPass(), &source_buffer_location, &destination_buffer_region, true);
}

void ROSE_MeshDestroy(ROSE_Mesh* mesh)
{
	if (mesh->_vertex_buffer) {
		SDL_ReleaseGPUBuffer(rose__device, mesh->_vertex_buffer);
	}

	if (mesh->_transfer_buffer) {
		SDL_ReleaseGPUTransferBuffer(rose__device, mesh->_transfer_buffer);
	}

	*mesh = (ROSE_Mesh) { 0 };
}

void ROSE_MeshDraw(ROSE_Mesh* mesh)
{
	if (rose_screen.minimized) {
		return;
	}

	if (!rose__render_pass) {
		ROSE_PANIC("ROSE_MeshDraw() called outside of a frame!");
	}

	if (mesh->_vertex_count == 0) {
		return;
	}

	float w = (mesh->scale / (float)rose_screen.width) * 2.0f;
	float h = -(mesh->scale / (float)rose_screen.height) * 2.0f;
	float x = -1.0f + ((mesh->xpos / (float)rose_screen.width) * 2.0f);
	float y = 1.0f - ((mesh->ypos / (float)rose_screen.height) * 2.0f);

	uInt32 uniform_size =
	sizeof(struct {
		float TransformMatrix[16];
		float TintColor[4];
	});

	uInt32 uniform_size_points =
	sizeof(struct {
		float TransformMatrix[16];
		float TintColor[4];
		float PixelDiameter;
	});

	typedef struct Uniform {
		float TransformMatrix[16];
		float TintColor[4];
		float PixelDiameter;
	} Uniform;

	const uInt32 color = mesh->color;
	float r = ((float)((uInt8)(color >> 24))) / 255.0f;
	float g = ((float)((uInt8)(color >> 16))) / 255.0f;
	float b = ((float)((uInt8)(color >> 8))) / 255.0f;
	float a = ((float)((uInt8)(color))) / 255.0f;

	Uniform uniform = {
		.TransformMatrix = {
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, 1, 0,
			x, y, 0, 1,
		},
		.TintColor = {
			r, g, b, a,
		},
		.PixelDiameter = mesh->diameter,
	};

	uInt32 primitive = mesh->primitive;
	const bool draw_triangles = (primitive & ROSE_TRIANGLES) != 0;
	const bool draw_lines     = (primitive & ROSE_LINES)     != 0;
	const bool draw_points    = (primitive & ROSE_POINTS)    != 0;

	if (draw_triangles) {
		ROSE__SDLBindGraphicsPipeline(rose__triangles_pipeline);
		SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), uniform_size);
		ROSE__SDLBindVertexBuffer(mesh->_vertex_buffer);
		SDL_DrawGPUPrimitives(rose__render_pass, mesh->_vertex_count, 1, 0, 0);
	}

	if (draw_lines) {
		ROSE__SDLBindGraphicsPipeline(rose__lines_pipeline);
		SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), uniform_size);
		ROSE__SDLBindVertexBuffer(mesh->_vertex_buffer);
		SDL_DrawGPUPrimitives(rose__render_pass, mesh->_vertex_count, 1, 0, 0);
	}

	if (draw_points) {
		ROSE__SDLBindGraphicsPipeline(rose__points_pipeline);
		SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), uniform_size_points);
		ROSE__SDLBindVertexBuffer(mesh->_vertex_buffer);
		SDL_DrawGPUPrimitives(rose__render_pass, mesh->_vertex_count, 1, 0, 0);
	}
}

void ROSE__MeshReserveVertexCapacity(ROSE_Mesh* mesh, uInt32 vertex_count)
{
	if (vertex_count <= mesh->_vertex_capacity) {
		return;
	}

	const uInt32 min_capacity = 32;
	uInt32 capacity = mesh->_vertex_capacity;
	if (capacity == 0) {
		capacity = min_capacity;
	}

	while (capacity < vertex_count) capacity *= 2;
	uInt32 vertex_buffer_size = capacity * (uInt32)sizeof(ROSE_Vertex2D);
	
	if (mesh->_vertex_buffer) {
		SDL_ReleaseGPUBuffer(rose__device, mesh->_vertex_buffer);
	}
	
	if (mesh->_transfer_buffer) {
		SDL_ReleaseGPUTransferBuffer(rose__device, mesh->_transfer_buffer);
	}

	SDL_GPUBufferCreateInfo buffer_create_info = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = vertex_buffer_size,
	};

	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = vertex_buffer_size,
	};

	mesh->_vertex_buffer = SDL_CreateGPUBuffer(rose__device, &buffer_create_info);
	mesh->_transfer_buffer = SDL_CreateGPUTransferBuffer(rose__device, &transfer_buffer_create_info);
	mesh->_vertex_capacity = capacity;
}

