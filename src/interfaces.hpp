#pragma once

namespace botshop
{

class Updateable
{
	virtual void step(float dt) = 0;
};

class Dynamic
{
	virtual Vec3 position() = 0;
	virtual Vec3 position(Vec3& pos) = 0;

	virtual Quat orientation() = 0;
	virtual Quat orientation(Quat& ori) = 0;

	virtual Vec3 velocity() = 0;
	virtual Vec3 velocity(Vec3& vel) = 0;

	virtual Vec3 force() = 0;
	virtual Vec3 force(Vec3& force) = 0;

	virtual Vec3 torque() = 0;
	virtual Vec3 torque(Vec3& torque) = 0;

	virtual dMass mass() = 0;
	virtual dMass mass(dMass mass) = 0;
};

}
