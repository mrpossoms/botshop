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

struct Framebuffer {
	GLuint color;
	GLuint depth;
	GLuint id;
};

class MaterialFactory
{
public:
	static Framebuffer create_framebuffer(int width, int height);
	static GLuint create_texture(int width, int height, GLenum format, void* data);
	static GLuint load_texture(std::string path);
	static Material* get_material(const std::string path);
};

}
