#include "form.hpp"
#include <GLFW/glfw3.h>

using namespace botshop;

Form::Form(dWorldID world, dSpaceID space, Model* mesh) : Body(world, space)
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		mesh->vert_count() * sizeof(Vertex),
		mesh->verts(),
		GL_STATIC_DRAW
	);

	this->mesh = mesh;
}


Form::~Form()
{
	glDeleteBuffers(1, &vbo);
}


void Form::draw(GLint world_uniform)
{
	glEnableVertexAttribArray(0);
	// glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(Vec3));

	mat4x4 world;
	matrix(world);

	glUniformMatrix4fv(world_uniform, 1, GL_FALSE, (GLfloat*)world);
	glDrawArrays(GL_TRIANGLES, 0, mesh->vert_count() / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
