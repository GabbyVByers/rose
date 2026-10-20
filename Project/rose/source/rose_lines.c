
/*
 *   Source File [rose_lines.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

ROSE_Lines ROSE_LinesCreate(ROSE_Vertex2D* vertices, uint32_t num_vertices)
{
	ROSE_Lines lines = ROSE_LinesCreateEmpty();
	ROSE_LinesUpdateVertices(&lines, vertices, num_vertices);
	return lines;
}

ROSE_Lines ROSE_LinesCreateEmpty(void)
{
	return (ROSE_Lines) {
		.scale = 1.0f,
		.xpos = 0.0f,
		.ypos = 0.0f,
		.color = ROSE_COLOR_WHITE,
		._num_vertices = 0,
		._vertex_capacity = 0,
		._vertex_buffer = NULL,
		._transfer_buffer = NULL,
	};
}

void ROSE_LinesUpdateVertices(ROSE_Lines* lines, ROSE_Vertex2D* vertices, uint32_t num_vertices)
{
	ROSE__LinesReserveVertexCapacity(lines, num_vertices);
	lines->_num_vertices = num_vertices;

	if (num_vertices == 0)
		return;

	uint32_t vertex_buffer_size = num_vertices * (uint32_t)sizeof(ROSE_Vertex2D);
	void* transfer_buffer_beginning = SDL_MapGPUTransferBuffer(rose__device, lines->_transfer_buffer, true);
	memcpy(transfer_buffer_beginning, vertices, (size_t)vertex_buffer_size);
	SDL_UnmapGPUTransferBuffer(rose__device, lines->_transfer_buffer);

	SDL_GPUTransferBufferLocation source_buffer_location = {
		.transfer_buffer = lines->_transfer_buffer,
	};

	SDL_GPUBufferRegion destination_buffer_region = {
		.buffer = lines->_vertex_buffer,
		.size = vertex_buffer_size,
	};

	SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(rose__device);
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
	SDL_UploadToGPUBuffer(copy_pass, &source_buffer_location, &destination_buffer_region, true);
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(command_buffer);
}

void ROSE_LinesDestroy(ROSE_Lines* lines)
{
	if (lines) {
		if (lines->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, lines->_vertex_buffer);
		if (lines->_transfer_buffer) SDL_ReleaseGPUTransferBuffer(rose__device, lines->_transfer_buffer);
		*lines = (ROSE_Lines) { 0 };
	}
}

void ROSE_LinesDraw(ROSE_Lines* lines)
{
	if (rose_screen.minimized || (lines->_num_vertices == 0))
		return;

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = lines->_vertex_buffer,
	};

	float ratio = (float)rose_screen.height / (float)rose_screen.width;
	float w = lines->scale * ratio;
	float h = lines->scale;
	float x = lines->xpos * ratio;
	float y = lines->ypos;

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
			lines->color.r,
			lines->color.g,
			lines->color.b,
			lines->color.a,
		},
	};

	SDL_BindGPUGraphicsPipeline(rose__render_pass, rose__lines_pipeline);
	SDL_PushGPUVertexUniformData(rose__command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(rose__render_pass, 0, &buffer_binding, 1);
	SDL_DrawGPUPrimitives(rose__render_pass, lines->_num_vertices, 1, 0, 0);
}

void ROSE__LinesReserveVertexCapacity(ROSE_Lines* lines, uint32_t num_vertices)
{
	if (num_vertices <= lines->_vertex_capacity)
		return;

	const uint32_t min_capacity = 32;
	uint32_t capacity = lines->_vertex_capacity;
	if (capacity == 0) {
		capacity = min_capacity;
	}

	while (capacity < num_vertices)
		capacity *= 2;

	uint32_t vertex_buffer_size = capacity * (uint32_t)sizeof(ROSE_Vertex2D);

	if (lines->_vertex_buffer) SDL_ReleaseGPUBuffer(rose__device, lines->_vertex_buffer);
	if (lines->_transfer_buffer) SDL_ReleaseGPUTransferBuffer(rose__device, lines->_transfer_buffer);

	SDL_GPUBufferCreateInfo buffer_create_info = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = vertex_buffer_size,
	};

	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = vertex_buffer_size,
	};

	lines->_vertex_buffer = SDL_CreateGPUBuffer(rose__device, &buffer_create_info);
	lines->_transfer_buffer = SDL_CreateGPUTransferBuffer(rose__device, &transfer_buffer_create_info);
	lines->_vertex_capacity = capacity;
}

