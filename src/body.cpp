#include "body.h"
#define ATTR_IS(name) if(!strcmp(attr.name(), (name)))

using namespace rapidxml;

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

Body::Body(dWorldID ode_world)
{
	this->ode_body = dBodyCreate(ode_world);
}

Body::Body(dWorldID ode_world, xml_node<>& node)
{
	this->ode_body = dBodyCreate(ode_world);


}

Body::~Body()
{

}
