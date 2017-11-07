#pragma once
#include "core.h"
#include "body.hpp"
#include "world.hpp"

namespace botshop
{

class Camera : public Body, public Viewer {
public:
	Camera(World& world, float fov, int frame_w, int frame_h);
	Viewer* view_projection(mat4x4 vp);
	Viewer* view(mat4x4 v);
	Viewer* projection(mat4x4 p);

private:
	mat4x4 _projection;
	int width, height;
};

}
