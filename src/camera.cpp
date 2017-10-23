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
	mat4x4 view;

	Vec3 pos = position();
	mat4x4_from_quat(view, orientation().v);
	mat4x4_translate(view, -pos.x, -pos.y, -pos.z);

	mat4x4_mul(vp, projection, view);
}
