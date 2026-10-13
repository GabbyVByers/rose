
/*
 *   Source File [rose_mesh.c]
 */

#define ROSE_EXTERN
#include "rose.h"
#include "rose_state.h"

void ROSE_CameraRotate(f32 axis_x, f32 axis_y, f32 axis_z, f32 radians)
{
	// normalize after rotating
}

void ROSE_CameraDirectionUp(f32* x, f32* y, f32* z)
{

}

void ROSE_CameraDirectionRight(f32* x, f32* y, f32* z)
{

}

void ROSE_CameraDirectionForward(f32* x, f32* y, f32* z)
{

}

void ROSE_CameraGetPosition(f32* x, f32* y, f32* z)
{
	*x = camera_px;
	*y = camera_py;
	*z = camera_pz;
}

void ROSE_CameraSetPosition(f32 x, f32 y, f32 z)
{
	camera_px = x;
	camera_py = y;
	camera_pz = z;
}

void ROSE_CameraSetFOV(f32 fov)
{
	camera_fov = fov;
}

