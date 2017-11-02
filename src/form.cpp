#include "form.hpp"
#include <GLFW/glfw3.h>

using namespace botshop;

Form::Form(World& world, Model* mesh) : Body(world)
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(
		GL_ARRAY_BUFFER,
		mesh->vert_count() * sizeof(Vertex),
		mesh->verts(),
		GL_STATIC_DRAW
	);

	Vertex v = mesh->verts()[0];
	printf("p %f %f %f \n", v.position[0], v.position[1], v.position[2]);
	printf("n %f %f %f \n", v.normal[0], v.normal[1], v.normal[2]);

	this->mesh = mesh;
}


Form::~Form()
{
	glDeleteBuffers(1, &vbo);
}


void Form::draw(DrawParams* params)
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));

	mat4x4 world;
	mat3x3 rot;
	matrix(world);
	rotation(rot);

	glUniformMatrix4fv(params->world_uniform, 1, GL_FALSE, (GLfloat*)world);
	glUniformMatrix3fv(params->norm_uniform,  1, GL_FALSE, (GLfloat*)rot);

	glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count());

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
