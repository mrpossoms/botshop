#pragma once
#include "core.h"
#include "body.hpp"

namespace botshop
{

class Camera : public Body {
public:
	Camera(dWorldID world, dSpaceID space, float fov, int frame_w, int frame_h);
	void view_projection(mat4x4 vp);

private:
	mat4x4 projection;
	int width, height;
};

}
