
/*
 *   Source File [rose_mesh.c]
 */

#define ROSE_EXTERN
#include "rose.h"

ROSE_Mesh ROSE_MeshCreate(ROSE_Vertex2D* vertices, uint32_t num_vertices)
{
	ROSE_Mesh mesh = ROSE_MeshCreateEmpty();
	ROSE_MeshUpdateVertices(&mesh, vertices, num_vertices);
	return mesh;
}

ROSE_Mesh ROSE_MeshCreateEmpty(void)
{
	return (ROSE_Mesh) {
		.shape = ROSE_TRIANGLES,
		.scale = 1.0f,
		.xpos = 0.0f,
		.ypos = 0.0f,
		.diameter = 1.0f,
		.color = ROSE_COLOR_WHITE,
		._num_vertices = 0,
		._vertex_capacity = 0,
		._vertex_buffer = NULL,
		._transfer_buffer = NULL,
	};
}

void ROSE_MeshUpdateVertices(ROSE_Mesh* mesh, ROSE_Vertex2D* vertices, uint32_t num_vertices)
{
	ROSE__MeshReserveVertexCapacity(mesh, num_vertices);
	mesh->_num_vertices = num_vertices;

	if (num_vertices == 0)
		return;

	uint32_t vertex_buffer_size = num_vertices * (uint32_t)sizeof(ROSE_Vertex2D);
	void* transfer_buffer_beginning = SDL_MapGPUTransferBuffer(rose__device, mesh->_transfer_buffer, true);
	memcpy(transfer_buffer_beginning, vertices, (size_t)vertex_buffer_size);
	SDL_UnmapGPUTransferBuffer(rose__device, mesh->_transfer_buffer);

	SDL_GPUTransferBufferLocation source_buffer_location = {
		.transfer_buffer = mesh->_transfer_buffer,
	};

	SDL_GPUBufferRegion destination_buffer_region = {
		.buffer = mesh->_vertex_buffer,
		.size = vertex_buffer_size,
	};

	SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(rose__device);
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
	SDL_UploadToGPUBuffer(copy_pass, &source_buffer_location, &destination_buffer_region, true);
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(command_buffer);
}

void ROSE_MeshDestroy(ROSE_Mesh* mesh)
{
	if (mesh) {
		if (mesh->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, mesh->_vertex_buffer);
		if (mesh->_transfer_buffer) SDL_ReleaseGPUTransferBuffer(rose__device, mesh->_transfer_buffer);
		*mesh = (ROSE_Mesh) { 0 };
	}
}

void ROSE_MeshDraw(ROSE_Mesh* mesh)
{
	if (rose_screen.minimized) return;
	if (!rose__render_pass) abort();

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = mesh->_vertex_buffer,
	};

	float ratio = (float)rose_screen.height / (float)rose_screen.width;
	float w = mesh->scale * ratio;
	float h = mesh->scale;
	float x = mesh->xpos * ratio;
	float y = mesh->ypos;

	uint32_t uniform_size =
	sizeof(struct {
		float TransformMatrix[16];
		float TintColor[4];
	});

	uint32_t uniform_size_points =
	sizeof(struct {
		float TransformMatrix[16];
		float TintColor[4];
		float PixelDiameter;
	});

	typedef struct Uniform {
		float TransformMatrix[16];
		float TintColor[4];
		float PixelDiameter;
		uint8_t _padding[4];
	} Uniform;

	Uniform uniform = {
		.TransformMatrix = {
			w, 0, 0, 0,
			0, h, 0, 0,
			0, 0, 1, 0,
			x, y, 0, 1,
		},
		.TintColor = {
			mesh->color.r,
			mesh->color.g,
			mesh->color.b,
			mesh->color.a,
		},
		.PixelDiameter = mesh->diameter,
	};

	switch (mesh->shape) {
		case (ROSE_TRIANGLES): {
			SDL_BindGPUGraphicsPipeline(rose__render_pass, rose__triangles_pipeline);
			SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), uniform_size);
		}
		break;
		case (ROSE_LINES): {
			SDL_BindGPUGraphicsPipeline(rose__render_pass, rose__lines_pipeline);
			SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), uniform_size);
		}
		break;
		case (ROSE_POINTS): {
			SDL_BindGPUGraphicsPipeline(rose__render_pass, rose__points_pipeline);
			SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), uniform_size_points);
		}
		break;
		default: break;
	}

	SDL_BindGPUVertexBuffers(rose__render_pass, 0, &buffer_binding, 1);
	SDL_DrawGPUPrimitives(rose__render_pass, mesh->_num_vertices, 1, 0, 0);
}

void ROSE_MeshCoordinateTransformPixelsToScreen(int32_t px, int32_t py, float* xpos, float* ypos)
{
	float screen_width = (float)rose_screen.width;
	float screen_height = (float)rose_screen.height;
	*xpos = ((2.0f * (float)px) + 1.0f - screen_width) / screen_height;
	*ypos = (screen_height - (2.0f * (float)py) - 1.0f) / screen_height;
}

void ROSE_MeshCoordinateTransformScreenToPixels(float xpos, float ypos, int32_t* px, int32_t* py)
{
	float screen_width = (float)rose_screen.width;
	float screen_height = (float)rose_screen.height;
	float float_px = ((xpos * screen_height) + screen_width) / 2.0f;
	float float_py = ((1.0f - ypos) * screen_height) / 2.0f;
	*px = (int32_t)SDL_floorf(float_px);
	*py = (int32_t)SDL_floorf(float_py);
}

void ROSE__MeshReserveVertexCapacity(ROSE_Mesh* mesh, uint32_t num_vertices)
{
	if (num_vertices <= mesh->_vertex_capacity)
		return;

	const uint32_t min_capacity = 32;
	uint32_t capacity = mesh->_vertex_capacity;
	if (capacity == 0) {
		capacity = min_capacity;
	}

	while (capacity < num_vertices) capacity *= 2;
	uint32_t vertex_buffer_size = capacity * (uint32_t)sizeof(ROSE_Vertex2D);
	if (mesh->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, mesh->_vertex_buffer);
	if (mesh->_transfer_buffer) SDL_ReleaseGPUTransferBuffer(rose__device, mesh->_transfer_buffer);

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

