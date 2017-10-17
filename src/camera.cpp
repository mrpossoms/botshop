#include "camera.hpp"

using namespace botshop;

Camera::Camera(dWorldID world,
	           dSpaceID space,
			   float fov,
			   int frame_w,
			   int frame_h) : Body(world, space)
{
	width = frame_w;
	height = frame_h;
	mat4x4_perspective(projection, fov, width / (float)height, 0.01, 1000);
}


void Camera::view_projection(mat4x4 vp)
{
	matrix(vp);
	mat4x4_mul(vp, vp, projection);
}
