#include "world.hpp"

using namespace botshop;

static void near_callback (void *data, dGeomID o1, dGeomID o2)
{
	World* world = (World*)data;
	if(o1 == world->ground ^ o1 == world->ground) return;

	dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);

    const int MAX_CONTACTS = 8;
    dContact contact[MAX_CONTACTS];

    int numc = dCollide (o1, o2, MAX_CONTACTS,
                        &contact[0].geom,
                        sizeof(dContact));

    for (int i=0; i<numc; i++) {
        contact[i].surface.mode = dContactApprox1;
        contact[i].surface.mu = 5;
        dJointID c = dJointCreateContact (world->ode_world, world->ode_contact_group, contact+i);
        dJointAttach (c, b1, b2);
	}
}


World::World()
{
	dInitODE2(0);
	ode_world = dWorldCreate();
	ode_space = dHashSpaceCreate(0);
	ode_contact_group = dJointGroupCreate (0);

	dWorldSetCFM (ode_world, 1e-5);
	dWorldSetAutoDisableFlag (ode_world, 1);
	dWorldSetGravity (ode_world, 0, 0, -.98);

	dWorldSetLinearDamping(ode_world, 0.00001);
	dWorldSetAngularDamping(ode_world, 0.005);
	dWorldSetMaxAngularSpeed(ode_world, 200);

	dWorldSetContactMaxCorrectingVel (ode_world, 0.1);
	dWorldSetContactSurfaceLayer (ode_world, 0.001);

	ground = dCreatePlane(ode_space, 0, 0, 1, 0);
	ground_mesh = new botshop::Plane(10);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(
		GL_ARRAY_BUFFER,
		ground_mesh->vert_count() * sizeof(Vertex),
		ground_mesh->verts(),
		GL_STATIC_DRAW
	);

}


World::~World()
{

}


World* World::operator+=(Dynamic& dynamic)
{
	dynamic.add_all();
	bodies.push_back(&dynamic);

	return this;
}


void World::step(float dt)
{
	dSpaceCollide (ode_space, this, &near_callback);
	dWorldQuickStep(ode_world, dt);
	dJointGroupEmpty(ode_contact_group);
}


void World::draw(DrawParams* params)
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));

	mat4x4 world;
	mat3x3 rot;
	mat4x4_identity(world);
	mat3x3_identity(rot);

	glUniformMatrix4fv(params->world_uniform, 1, GL_FALSE, (GLfloat*)world);
	glUniformMatrix3fv(params->norm_uniform, 1, GL_FALSE, (GLfloat*)rot);

	glDrawArrays(GL_TRIANGLES, 0, ground_mesh->vert_count());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	for(Dynamic* body : bodies)
	{

		Drawable* form = dynamic_cast<Drawable*>(body);
		if(!form) continue;

		form->draw(params);
	}
}
