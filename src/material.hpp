#pragma once

#include "core.h"

namespace botshop
{

union Material {
	struct {
		GLuint color, normal, specular;
	} textures;
	GLuint v[3];

	void use(GLint* material_uniforms);
};

class MaterialFactory
{
public:
	static Material* get_material(const std::string path);
};

}
