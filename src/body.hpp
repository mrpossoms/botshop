#pragma once
#include "core.h"
#include "geo.hpp"

using namespace rapidxml;

namespace botshop
{

class Body : Dynamic
{
	protected:
		// used to group objects and speed up collision detection. A space will
		// be generated by the root Body and propogated to its children
		dSpaceID ode_body_space;

	public:
		Body(dWorldID ode_world);
		Body(dWorldID ode_world, xml_node<>& node);
		~Body();

		dBodyID ode_body;
		dGeomID ode_geo;

		STLModel* model;

		std::vector<Body*> welded_children;
		std::vector<Body*> jointed_children;
		std::vector<dJointID> joints;

		void weld(Body* body);
		void joint_caster(Body* body, Vec3& anchor);

		virtual Vec3 position();
		virtual Vec3 position(Vec3& pos);

		virtual Quat orientation();
		virtual Quat orientation(Quat& ori);

		virtual Vec3 velocity();
		virtual Vec3 velocity(Vec3& vel);

		virtual Vec3 force();
		virtual Vec3 force(Vec3& force);

		virtual Vec3 torque();
		virtual Vec3 torque(Vec3& torque);

		virtual dMass mass();
		virtual dMass mass(dMass mass);
};

}
