#include "body.hpp"

using namespace botshop;

//------------------------------------------------------------------------------
//    __  __     _   _            _
//   |  \/  |___| |_| |_  ___  __| |___
//   | |\/| / -_)  _| ' \/ _ \/ _` (_-<
//   |_|  |_\___|\__|_||_\___/\__,_/__/
//
Body::Body(World& world)
{
	this->world = world.ode_world;
	this->space = world.ode_space;

	this->ode_body = dBodyCreate(world.ode_world);
}

//------------------------------------------------------------------------------

Body::~Body()
{
	dGeomDestroy(ode_geo);
	dBodyDestroy(ode_body);
}
//------------------------------------------------------------------------------

void Body::add_all()
{
	for(Body* body : welded_children)
	{
		body->add_all();
	}

	for(Joint* joint : jointed_children)
	{
		joint->body->add_all();
	}
}
//------------------------------------------------------------------------------

void Body::remove_all()
{

}
//------------------------------------------------------------------------------

Body* Body::attach(Body* body)
{
	return NULL;
}
//------------------------------------------------------------------------------

Body* Body::attach(const Joint* joint)
{
	dJointAttach(joint->ode_joint, ode_body, joint->body->ode_body);
	welded_children.push_back(joint->body);

	return this;
}
//------------------------------------------------------------------------------

Body* Body::is_a_box(float width, float height, float length)
{
	ode_geo = dCreateBox(0, length, width, height);
	dMassSetBox(&ode_mass, 1, length, width, height);
	dGeomSetBody(ode_geo, ode_body);
	dSpaceAdd(space, ode_geo);

	return this;
}
//------------------------------------------------------------------------------

Body* Body::is_a_sphere(float radius)
{
	ode_geo = dCreateSphere(0, radius);
	dMassSetSphere(&ode_mass, 1, radius);
	dGeomSetBody(ode_geo, ode_body);
	dSpaceAdd(space, ode_geo);

	return this;
}
//------------------------------------------------------------------------------
Body* Body::is_a_mesh(Model& model)
{
	ode_geo = model.create_collision_geo(space);
	dMassSetTrimesh(&ode_mass, 1, ode_geo);
	dGeomSetBody(ode_geo, ode_body);
	return this;
}
//------------------------------------------------------------------------------
Body* Body::is_a_mesh(Model* model)
{
	ode_geo = model->create_collision_geo(space);
	dMassSetTrimesh(&ode_mass, 1, ode_geo);
	dGeomSetBody(ode_geo, ode_body);
	return this;
}
//------------------------------------------------------------------------------

Vec3 Body::position()
{
	const dReal* p = dBodyGetPosition(ode_body);
	Vec3 pos(p[0], p[1], p[2]);
	return pos;
}
//------------------------------------------------------------------------------

Body* Body::position(Vec3& pos)
{
	dBodySetPosition(ode_body, pos.x, pos.y, pos.z);
	return this;
}
//------------------------------------------------------------------------------

Body* Body::position(float x, float y, float z)
{
	Vec3 v(x, y, z);
	return position(v);
}
//------------------------------------------------------------------------------
Quat Body::orientation()
{
	const dReal* q = dBodyGetQuaternion(ode_body);
	Quat Q(q[1], q[2], q[3], q[0]);
	return Q;
}

//------------------------------------------------------------------------------
Body* Body::orientation(Quat& ori)
{
	dQuaternion Q = { ori.w, ori.x, ori.y, ori.z };
	dBodySetQuaternion(ode_body, Q);
	return this;
}

//------------------------------------------------------------------------------
Vec3 Body::velocity()
{
	const dReal* v = dBodyGetLinearVel(ode_body);
	Vec3 vel(v[0], v[1], v[2]);
	return vel;
}

//------------------------------------------------------------------------------
Body* Body::velocity(Vec3& vel)
{
	return this;
}

//------------------------------------------------------------------------------
Vec3 Body::force()
{
	const dReal* f = dBodyGetForce(ode_body);
	Vec3 force(f[0], f[1], f[2]);
	return force;
}

//------------------------------------------------------------------------------
Body* Body::force(Vec3& f)
{
	dBodyAddRelForce(ode_body, f.x, f.y, f.z);
	return this;
}

//------------------------------------------------------------------------------
Body* Body::force(float x, float y, float z)
{
	dBodyAddRelForce(ode_body, x, y, z);
	return this;
}

//------------------------------------------------------------------------------
Vec3 Body::torque()
{
	const dReal* t = dBodyGetTorque(ode_body);
	Vec3 torque(t[0], t[1], t[2]);
	return torque;
}

//------------------------------------------------------------------------------
Body* Body::torque(Vec3& t)
{
	dBodyAddRelTorque(ode_body, t.x, t.y, t.z);
	return this;
}

//------------------------------------------------------------------------------
Body* Body::torque(float x, float y, float z)
{
	dBodyAddRelTorque(ode_body, x, y, z);
	return this;
}

//------------------------------------------------------------------------------
dMass Body::mass()
{
	return ode_mass;
}

//------------------------------------------------------------------------------
Body* Body::mass(float m)
{
	dMassSetTrimesh(&ode_mass, 1, ode_geo);
	dMassAdjust(&ode_mass, m);
	dBodySetMass(ode_body, &ode_mass);
	return this;
}

//------------------------------------------------------------------------------

Body* Body::operator+(const Joint* joint)
{
	return attach(joint);
}

//------------------------------------------------------------------------------

void Body::matrix(mat4x4 world)
{
	Vec3 pos = position();
	mat4x4_from_quat(world, orientation().v);
	mat4x4_translate(world, pos.x, pos.y, pos.z);
}
//------------------------------------------------------------------------------

void Body::rotation(mat3x3 rotation)
{
	mat3x3_from_quat(rotation, orientation().v);
}
//------------------------------------------------------------------------------

Joint::Joint(Body& body, dJointID joint)
{
	ode_joint = joint;
	this->body = &body;
}

//------------------------------------------------------------------------------

Joint* Joint::at(Vec3 a)
{
	dJointSetHinge2Anchor(ode_joint, a.x, a.y, a.z);
	return this;
}
//------------------------------------------------------------------------------

Joint* Joint::wheel(Body& body, Vec3 steer_axis, Vec3 axle_axis)
{
	Joint* wheel = new Joint(body, dJointCreateHinge2(body.world, 0));
	dJointSetHinge2Axis1(wheel->ode_joint, steer_axis.x, steer_axis.y, steer_axis.z);
	dJointSetHinge2Axis2(wheel->ode_joint, axle_axis.x,  axle_axis.y,  axle_axis.z);
	dJointSetHinge2Param(wheel->ode_joint, dParamSuspensionERP, 0.4);
	dJointSetHinge2Param(wheel->ode_joint, dParamSuspensionCFM, 0.8);

	return wheel;
}
