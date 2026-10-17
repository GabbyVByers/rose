
/*
 *   Source File [rose_mesh.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

static void mmult(f32* a, f32* b, f32* result)
{
	for (u32 i = 0; i < 4; i++)
	{
		for (u32 j = 0; j < 4; j++)
		{
			f32 sum = 0.0f;

			for (u32 k = 0; k < 4; k++)
			{
				sum += a[(i * 4) + k] * b[(k * 4) + j];
			}

			result[(i * 4) + j] = sum;
		}
	}
}

ROSE_Mesh ROSE_MeshCreate(void)
{
	return (ROSE_Mesh) {
		.scale = 1.0f,
		.px = 0.0f,
		.py = 0.0f,
		.pz = 0.0f,
		.qw = 1.0f,
		.qx = 0.0f,
		.qy = 0.0f,
		.qz = 0.0f,
		.color = ROSE_ColorROSE_COLOR_WHITE,
		.count = 0,
		.vertices = NULL,
		.texture = NULL,
	};
}

void ROSE_MeshDestroy(ROSE_Mesh* mesh)
{
	if (mesh->vertices)
	{
		SDL_ReleaseGPUBuffer(device, mesh->vertices);
	}

	if (mesh->texture)
	{
		SDL_ReleaseGPUTexture(device, mesh->texture);
	}
}

void ROSE_MeshUploadVertices(ROSE_Mesh* mesh, ROSE_Vertex3D* vertices, u32 count)
{
	if (mesh->vertices)
	{
		SDL_ReleaseGPUBuffer(device, mesh->vertices);
	}

	mesh->count = count;
	mesh->vertices = ROSE_SDLCreateVertexBuffer(vertices, count, sizeof(ROSE_Vertex3D));
}

void ROSE_MeshUploadTexture(ROSE_Mesh* mesh, ROSE_Image* image)
{
	if (mesh->texture)
	{
		SDL_ReleaseGPUTexture(device, mesh->texture);
	}

	mesh->texture = ROSE_SDLCreateTexture(image);
}

void ROSE_MeshDraw(ROSE_Mesh* mesh, ROSE_Camera* camera)
{
	if (minimized)
	{
		return;
	}

	SDL_GPUBufferBinding buffer_binding = {
		.buffer = mesh->vertices,
	};

	SDL_GPUTextureSamplerBinding texture_binding = {
		.texture = (mesh->texture) ? (mesh->texture) : (dummy_texture),
		.sampler = sampler,
	};

	f32 model[16] = { 0 };
	f32 view[16] = { 0 };

	{
		f32 px = mesh->px;
		f32 py = mesh->py;
		f32 pz = mesh->pz;

		f32 translation[16] = {
			1, 0, 0, px,
			0, 1, 0, py,
			0, 0, 1, pz,
			0, 0, 0, 1,
		};

		f32 w = mesh->qw;
		f32 x = mesh->qx;
		f32 y = mesh->qy;
		f32 z = mesh->qz;

		f32 rotation[16] = {
			1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w), 0,
			2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w), 0,
			2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y), 0,
			0, 0, 0, 1,
		};

		f32 s = mesh->scale;

		f32 scalar[16] = {
			s, 0, 0, 0,
			0, s, 0, 0,
			0, 0, s, 0,
			0, 0, 0, 1,
		};

		f32 temp[16] = { 0 };
		mmult(translation, rotation, temp);
		mmult(temp, scalar, model);
	}

	{
		f32 px = camera->px;
		f32 py = camera->py;
		f32 pz = camera->pz;

		f32 translation[16] = {
			1, 0, 0, -px,
			0, 1, 0, -py,
			0, 0, 1, -pz,
			0, 0, 0, 1,
		};

		f32 w =  camera->qw;
		f32 x = -camera->qx;
		f32 y = -camera->qy;
		f32 z = -camera->qz;

		f32 rotation[16] = {
			1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w), 0,
			2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w), 0,
			2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y), 0,
			0, 0, 0, 1,
		};

		mmult(rotation, translation, view);
	}

	f32 asp = (f32)screen_width / (f32)screen_height;
	f32 f = 1.0f / tanf(camera->fov * 0.5f);
	f32 n = camera->near_plane;

	f32 project[16] = {
		f / asp, 0, 0, 0,
		0, f, 0, 0,
		0, 0, 1, -n,
		0, 0, 1, 0,
	};

	f32 MVP[16] = { 0 };
	f32 temp[16] = { 0 };
	mmult(project, view, temp);
	mmult(temp, model, MVP);

	typedef struct Uniform {
		f32 TransformMatrix[16];
	} Uniform;

	Uniform uniform = { 0 };

	for (u32 i = 0; i < 4; i++)
	{
		for (u32 j = 0; j < 4; j++)
		{
			uniform.TransformMatrix[(i * 4) + j] = MVP[(j * 4) + i];
		}
	}

	SDL_BindGPUGraphicsPipeline(render_pass, mesh_pipeline);
	SDL_PushGPUVertexUniformData(command_buffer, 0, ((void*)&uniform), sizeof(uniform));
	SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_binding, 1);
	SDL_BindGPUFragmentSamplers(render_pass, 0, &texture_binding, 1);
	SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);
}

//void ROSE_MeshResetRotation(ROSE_Mesh* mesh)
//{
//	mesh->qw = 1.0f;
//	mesh->qx = 1.0f;
//	mesh->qy = 1.0f;
//	mesh->qz = 1.0f;
//}
//
//void ROSE_MeshRotate(ROSE_Mesh* mesh, f32 axis_x, f32 axis_y, f32 axis_z, f32 radians)
//{
//	// normalize after rotating
//}
//
//void ROSE_MeshDirectionUp(ROSE_Mesh* mesh, f32* x, f32* y, f32* z)
//{
//
//}
//
//void ROSE_MeshDirectionRight(ROSE_Mesh* mesh, f32* x, f32* y, f32* z)
//{
//
//}
//
//void ROSE_MeshDirectionForward(ROSE_Mesh* mesh, f32* x, f32* y, f32* z)
//{
//
//}

