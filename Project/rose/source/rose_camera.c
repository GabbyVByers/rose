
/*
 *   Source File [rose_mesh.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

ROSE_Camera ROSE_CameraInit(void)
{
	return (ROSE_Camera) {
		.fov = 1.0f,
		.near_plane = 0.1f,
		.px = 0.0f,
		.py = 0.0f,
		.pz = 0.0f,
		.qw = 1.0f,
		.qx = 0.0f,
		.qy = 0.0f,
		.qz = 0.0f,
	};
}

void ROSE_CameraRotate(ROSE_Camera* camera, f32 axis_x, f32 axis_y, f32 axis_z, f32 radians)
{
	// normalize after rotating
}

void ROSE_CameraDirectionUp(ROSE_Camera* camera, f32* x, f32* y, f32* z)
{

}

void ROSE_CameraDirectionRight(ROSE_Camera* camera, f32* x, f32* y, f32* z)
{

}

void ROSE_CameraDirectionForward(ROSE_Camera* camera, f32* x, f32* y, f32* z)
{

}

