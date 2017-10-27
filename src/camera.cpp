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
	mat4x4_perspective(_projection, fov, width / (float)height, 0.01, 1000);
}


Camera* Camera::view_projection(mat4x4 vp)
{
	mat4x4 view;

	Vec3 pos = position();
	mat4x4_from_quat(view, orientation().v);
	mat4x4_translate_in_place(view, -pos.x, -pos.y, -pos.z);

	mat4x4_mul(vp, _projection, view);

	return this;
}


Camera* Camera::view(mat4x4 v)
{
	Vec3 pos = position();
	mat4x4_from_quat(v, orientation().v);
	mat4x4_translate_in_place(v, -pos.x, -pos.y, -pos.z);

	return this;
}


Camera* Camera::projection(mat4x4 p)
{
	mat4x4_dup(p, _projection);
	
	return this;
}
