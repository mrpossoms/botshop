#pragma once
#include "core.h"
#include "geo.hpp"

namespace botshop
{

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
	void draw(DrawParams* params);

	dWorldID ode_world;
	dSpaceID ode_space;
	dJointGroupID ode_contact_group;
	dGeomID ground;

	World* operator+=(Dynamic& dynamic);

private:
	std::vector<Field> feilds;
	std::vector<Field> forces;

	std::vector<Dynamic*> bodies;

	GLuint vbo;
	Model* ground_mesh;
};

}
