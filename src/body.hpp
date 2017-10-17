#pragma once
#include "core.h"
#include "geo.hpp"

extern dWorldID ODE_CURRENT_WORLD;

namespace botshop
{

class Joint;
class Body;

class Body : Dynamic
{
	protected:
		// used to group objects and speed up collision detection. A space will
		// be generated by the root Body and propogated to its children
		dSpaceID ode_body_space;

	public:
		Body(dWorldID world, dSpaceID space);
		~Body();

		dWorldID world;
		dSpaceID space;

		dBodyID ode_body;
		dGeomID ode_geo;
		dMass   ode_mass;

		Body* parent;
		STLModel* model;

		std::vector<Body*> welded_children;
		std::vector<Joint*> jointed_children;
		std::vector<dJointID> joints;

		Body* add_all();
		Body* remove_all();

		Body* attach(const Joint* joint);

		Body* is_a_box(float width, float height, float length);
		Body* is_a_sphere(float radius);
		Body* is_a_mesh(STLModel& model);

		// Dynamic Interface
		Vec3 position();
		Body* position(Vec3& pos);
		Body* position(float x, float y, float z);

		Quat orientation();
		Body* orientation(Quat& ori);

		Vec3 velocity();
		Body* velocity(Vec3& vel);

		Vec3 force();
		Body* force(Vec3& force);

		Vec3 torque();
		Body* torque(Vec3& torque);

		dMass mass();
		Body* mass(float mass);

		void matrix(mat4x4 world);

		Body* operator+(const Joint* joint);
};


class Joint
{
public:
	Joint(Body& body, dJointID joint);

	dJointID ode_joint;
	Body* body;

	Joint* at(Vec3 anchor);

	static Joint* wheel(Body& body, Vec3 steer_axis, Vec3 axle_axis);
};

}
