#pragma once

#include "core.h"
#include "geo.hpp"

namespace botshop
{

class Field
{
	virtual int  id() = 0;
	virtual Vec3 vectorAt(Vec3& point, float t) = 0;
};

class World : public Updateable, public Drawable, public Scene
{
public:
	World();
	~World();

	void                    step(float dt);
	void                    draw(DrawParams* params);
	std::vector<Drawable*>& drawables();

	dWorldID ode_world;
	dSpaceID ode_space;
	dJointGroupID ode_contact_group;
	dGeomID ground;

	World* operator+=(Dynamic& dynamic);

private:
	std::vector<Field> feilds;
	std::vector<Field> forces;

	std::vector<Dynamic*> dynamic_set;
	std::vector<Drawable*> drawable_set;

	GLuint vbo;
	Model* ground_mesh;
};

}
