#pragma once

#include "core.h"
#include "body.hpp"
#include "geo.hpp"

namespace botshop {

class Form : public Body
{
public:
	Form(dWorldID world, dSpaceID space, Model* mesh);
	~Form();

	void draw(GLint world_uniform, GLint norm_uniform);
	// void draw_test(GLint world_uniform);

private:
	GLuint vbo;
	Model* mesh;
};

}
