#pragma once

#include "core.h"
#include "body.hpp"
#include "geo.hpp"
#include "world.hpp"

namespace botshop {

class Form : public Body
{
public:
	Form(World& world, Model* mesh);
	~Form();

	void draw(GLint world_uniform, GLint norm_uniform);

private:
	GLuint vbo;
	Model* mesh;
};

}
