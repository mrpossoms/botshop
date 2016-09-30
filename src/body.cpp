#include "body.h"
#define ATTR_IS(name) if(!strcmp(attr->name(), (name)))
#define ATTR_VALUE_IS(name) if(!strcmp(attr->value(), (name)))
#define CHILD_IS_NAMED(name) if(!strcmp(child->name(), (name)))
#define EACH_NUMBER_IN(STR, N) for(char i=0, *number=strtok((STR), ","); i++<(N); number=strtok(NULL, ","))
#define EACH_CHILD(NODE) for(xml_node<>* child=(NODE).first_node();child;child=child->next_sibling())
#define EACH_ATTR(NODE) for(xml_attribute<>* attr = (NODE).first_attribute();attr;attr=attr->next_attribute())

using namespace rapidxml;

const char* HINGE_KEYS[] = {
	"hinge",
	"piston",
	"motor",
	"hinge2",
	NULL
};

dJointId (*HINGE_CREATORS[])(dWorldID, dJointGroupID) = {
	dJointCreateHinge,
	dJointCreatePiston,
	dJointCreateAMotor,
	dJointCreateHinge2
};

void (*HINGE_AXES[][3])(dJointID, dReal, dReal, dReal) = {
	{ dJointSetHingeAnchor },
	{ dJointSetPistonAxis },
	{ _motor_axis_0, _motor_axis_1, _motor_axis_2 },
	{ dJointSetHinge2Axis1, dJointSetHinge2Axis2 }
};

void (*HINGE_ANCHORS[])(dJointID, dReal, dReal, dReal) = {
	dJointSetHingeAnchor,
	dJointSetPistonAnchor,
	NULL,
	dJointSetHinge2Anchor
};

static Quat _quat_rule(xml_node<>* node)
{
	Quat q;
	char num_str[32] = {};

	EACH_NUMBER_IN(node->value(), 4)
	{
		memcpy(num_str, number, node->value_size());
		v.v[i] = atof(num_str);
	}

	return q;
}
//------------------------------------------------------------------------------
static Vec3 _vec3_rule(xml_node<>* node)
{
	Vec3 v;
	char num_str[32] = {};

	EACH_NUMBER_IN(node->value(), 3)
	{
		memcpy(num_str, number, node->value_size());
		v.v[i] = atof(num_str);
	}

	return v;
}
//------------------------------------------------------------------------------
static double _scalar_rule(xml_node<>* node)
{
	return atof(node->value());
}
//------------------------------------------------------------------------------
static int _phys_rule(Body& body, xml_node<>& node)
{
	EACH_CHILD(node)
	{
		CHILD_IS_NAMED("mass")
		{
			dMass m;
			dMassAdjust(&m, atoi(child->value()));
			dBodySetMass(body.ode_body, &m);
		}
		else CHILD_IS_NAMED("friction")
		{
			// TODO
		}
		else CHILD_IS_NAMED("staticFriction")
		{
			// TODO
		}
		else CHILD_IS_NAMED("restitution")
		{
			// TODO
		}
	}

	return 0;
}
//------------------------------------------------------------------------------
static int _model_rule(Body& body, xml_node<>& node)
{
	EACH_ATTR(node)
	{
		ATTR_IS("src")
		{
			body.model = STLFactory.get_model(attr->value());
			assert(body.model);
		}
	}

	return 0;
}
//------------------------------------------------------------------------------
static void _motor_axis_0(dJointID joint, dReal x, dReal y, dReal z)
{
	dJointSetAMotorAxis(joint, 0, 0, x, y, z);
}
//------------------------------------------------------------------------------
static void _motor_axis_1(dJointID joint, dReal x, dReal y, dReal z)
{
	dJointSetAMotorAxis(joint, 1, 0, x, y, z);
}
//------------------------------------------------------------------------------
static void _motor_axis_2(dJointID joint, dReal x, dReal y, dReal z)
{
	dJointSetAMotorAxis(joint, 2, 0, x, y, z);
}
//------------------------------------------------------------------------------
static int _actuator_rule(Body& body, xml_node<>& node)
{
	dJointID joint;
	int actuator_type = 0;

	// figure out what we are
	EACH_ATTR(node)
	{
		ATTR_IS("type")
		{
			for(int i = 0; HINGE_KEYS[i]; ++i)
			{
				ATTR_VALUE_IS(HINGE_KEYS[i])
				{
					actuator_type = i;
					break;
				}
			}

			joint = HINGE_CREATORS[actuator_type](__WURLD__, 0); // TODO
		}
	}

	int axis_idx = 0;
	EACH_CHILD(node)
	{
		CHILD_IS_NAMED("axis")
		{
			Vec3 axis = _vec3_rule(child);

			HINGE_AXES[actuator_type][axis_idx](
				joint,
				axis.x, axis.y, axis.z
			);

			axis_idx++;
		}
		else CHILD_IS_NAMED("position")
		{
			Vec3 anchor = _vec3_rule(child);

			HINGE_ANCHORS[actuator_type](
				joint,
				anchor.x, anchor.y, anchor.z
			);
		}
		else CHILD_IS_NAMED("body")
		{
			Body* new_body = _body_rule(child);

			dJointAttach(joint, body.ode_body, new_body->ode_body);
		}
	}

	return 0;
}
//------------------------------------------------------------------------------
static Body* _body_rule(xml_node<>* node)
{
	return 0;
}
//------------------------------------------------------------------------------
static int _body_proc_attribute(Body& body, xml_attribute<>& attr)
{
	ATTR_IS("mass")
	{
		dMass m;
		dMassAdjust(&m, atoi(attr.value()));
		dBodySetMass(body.ode_body, &m);
	}
	else ATTR_IS("model")
	{
		int fd = open(attr.value(), O_RDONLY);
		body.model = new STLModel(fd);
		assert(body.model);
		close(fd);
	}
	else ATTR_IS("position")
	{
		const char* delim = ",";
		char num_str[32] = {};
		Vec3 position;

		for(char i = 0, *number = strtok(attr.value(), delim);
		    i++ < 3;
		    number = strtok(NULL, delim))
		{
			memcpy(num_str, number, attr.value_size());
			position.v[i] = atof(num_str);
		}

		dBodySetPosition(body.ode_body, position.x, position.y, position.z);
	}
	else ATTR_IS("orientation")
	{
		const char* delim = ",";
		char num_str[32] = {};
		Quat orientation;

		for(char i = 0, *number = strtok(attr.value(), delim);
			i++ < 4;
			number = strtok(NULL, delim))
		{
			memcpy(num_str, number, attr.value_size());
			orientation[i] = atof(num_str);
		}

		dBodySetQuaternion(body.ode_body, orientation);
	}

	return 0;
}
//------------------------------------------------------------------------------
Body::Body(dWorldID ode_world)
{
	this->ode_body = dBodyCreate(ode_world);
}
//------------------------------------------------------------------------------
Body::Body(dWorldID ode_world, xml_node<>& node)
{
	this->ode_body = dBodyCreate(ode_world);


}

Body::~Body()
{

}
