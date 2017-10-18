#pragma once

#include "core.h"
#include "body.hpp"
#include "geo.hpp"

namespace botshop {

class Form : public Body, Drawable
{
public:
	Form(dWorldID world, dSpaceID space, Model* mesh);
	~Form();

	void draw(GLint world_uniform);

private:
	GLuint vbo;
	unsigned int vert_count;
};

}
