#include "form.hpp"
#include <GLFW/glfw3.h>

using namespace botshop;

Form::Form(dWorldID world, dSpaceID space, Model* mesh) : Body(world, space)
{
	vert_count = mesh->tri_count * 3;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		vert_count * sizeof(STLVert),
		mesh->all_verts,
		GL_STATIC_DRAW
	);
}


Form::~Form()
{
	glDeleteBuffers(1, &vbo);
}


void Form::draw(GLint world_uniform)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(STLVert), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(STLVert), (void*)sizeof(Vec3));

	mat4x4 world;
	matrix(world);

	glUniformMatrix4fv(world_uniform, 1, GL_FALSE, (GLfloat*)world);
	glDrawArrays(GL_TRIANGLES, 0, vert_count / 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
