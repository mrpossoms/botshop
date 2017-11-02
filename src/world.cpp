#include "world.hpp"

using namespace botshop;

World::World()
{
	dInitODE2(0);
	ode_world = dWorldCreate();
	ode_space = dHashSpaceCreate(0);

	dWorldSetGravity (ode_world, 0, 0, -9.8);

	ground = dCreatePlane (ode_space,0,0,1,0);
	ground_mesh = new botshop::Plane(1000);

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


void World::step(float dt)
{
	dWorldStep(ode_world, dt);
}


void World::draw(GLint world_uniform, GLint norm_uniform)
{
	ground_mesh->draw(world_uniform, norm_uniform);
}
