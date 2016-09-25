#pragma once
#include "core.h"

namespace botshop
{

class Field
{
	virtual int id() = 0;
	virtual Vec3 vectorAt(Vec3& point, float t) = 0;
};

class World : Updateable
{
	private:
		dWorldID ode_world;
		vector<Field> feilds;
		vector<Field> forces;

	public:
		World();
		~World();

		void step(float dt);
};

}
