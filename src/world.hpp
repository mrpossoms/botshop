#pragma once
#include "core.h"

namespace botshop
{

class Form;

class Field
{
	virtual int id() = 0;
	virtual Vec3 vectorAt(Vec3& point, float t) = 0;
};

class World : Updateable, Drawable
{
public:
	World();
	~World();

	void step(float dt);
	void draw(GLint world_uniform, GLint norm_uniform);

	dWorldID ode_world;
	dSpaceID ode_space;

private:
	dGeomID ground;
	std::vector<Field> feilds;
	std::vector<Field> forces;

	GLuint vbo;
	Model* ground_mesh;
};

}
