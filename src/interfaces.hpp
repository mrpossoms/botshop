#pragma once

#include <GLFW/glfw3.h>

namespace botshop
{

class Updateable
{
public:
	virtual void step(float dt) = 0;
};

class Dynamic
{
public:
	virtual Vec3 position() = 0;
	virtual Quat orientation() = 0;
	virtual Vec3 velocity() = 0;
	virtual Vec3 force() = 0;
	virtual Vec3 torque() = 0;
	virtual dMass mass() = 0;
	virtual void matrix(mat4x4 world) = 0;
};

class Drawable
{
public:
	virtual void draw(GLint world_uniform, GLint norm_uniform) = 0;
};

}
