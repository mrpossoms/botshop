#pragma once
#include "core.h"
#include "body.hpp"

namespace botshop
{

class Camera : public Body {
public:
	Camera(dWorldID world, dSpaceID space, float fov, int frame_w, int frame_h);
	Camera* view_projection(mat4x4 vp);
	Camera* view(mat4x4 v);
	Camera* projection(mat4x4 p);

private:
	mat4x4 _projection;
	int width, height;
};

}
