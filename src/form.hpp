#pragma once

#include "core.h"
#include "geo.hpp"
#include "body.hpp"


namespace botshop
{

class Form : public Body, public Drawable
{
public:
	Form(World& world, Model* mesh);
	~Form();

	void draw(DrawParams* params);

private:
	GLuint vbo;
	Model* mesh;
};

}
