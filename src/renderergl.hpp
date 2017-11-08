#pragma once

#include "core.h"
#include "material.hpp"

namespace botshop
{


struct EnvironmentMap {
	Framebuffer framebuffer;
	GLuint map;

	EnvironmentMap(int res);
	void render_to(GLenum face);
};

struct Shader {
	Shader(GLint vertex, GLint frag);

	GLint program;
	DrawParams draw_params;
};

class RendererGL : public Renderer
{
public:
	RendererGL(std::string data_path);

	virtual void draw(Viewer* viewer, Scene* scene);

	GLFWwindow* win;
private:
	void draw_scene(Scene* scene);
	void draw_to(EnvironmentMap* env, Scene* scene, Vec3 at_location);

	Shader* pbr_shader;
	Shader* simple_shader;
	DrawParams draw_params;
	EnvironmentMap* env;
};

}
