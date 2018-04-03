#pragma once

#include <GLFW/glfw3.h>

namespace botshop
{

class Updateable
{
public:
	virtual void step(float dt) = 0;
};


class Attachable
{
public:
	// virtual void on_attached(Attachable* parent) = 0;
	void on_attach(const Attachable* child);
};


class Dynamic
{
public:
	virtual void add_all() = 0;
	virtual void remove_all() = 0;
	virtual Vec3 position() = 0;
	virtual Quat orientation() = 0;
	virtual Vec3 velocity() = 0;
	virtual Vec3 force() = 0;
	virtual Vec3 torque() = 0;
	virtual dMass mass() = 0;
	virtual void matrix(mat4x4 world) = 0;
};


class Viewer
{
public:
	virtual Viewer* view_projection(mat4x4 vp) = 0;
	virtual Viewer* view(mat4x4 v) = 0;
	virtual Viewer* projection(mat4x4 p) = 0;
};


class Drawable
{
public:
	virtual void draw(DrawParams* params) = 0;
};


class Scene
{
public:
	virtual void draw(DrawParams* params) = 0;
	virtual std::vector<Drawable*>& drawables() = 0;
};


class Renderer
{
public:
	virtual bool should_close() = 0;
	virtual void draw(Viewer* viewer, Scene* scene) = 0;
};

}
