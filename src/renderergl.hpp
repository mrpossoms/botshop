#pragma once

#include "core.h"
#include "material.hpp"

namespace seen
{

class Sky : public Drawable {
public:
    Sky();
    ~Sky();

    void draw(DrawParams* params);
private:
    GLuint vbo;
    int vertices;
};


struct EnvironmentMap {
	Framebuffer framebuffer;
	GLuint map;

	EnvironmentMap(int res);
	void render_to(GLenum face);

	int size;
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

	virtual bool should_close();
	virtual void draw(Viewer* viewer, Scene* scene);

	GLFWwindow* win;
	int width, height;
private:
	void draw_scene(Scene* scene);
	void draw_to(EnvironmentMap* env, Scene* scene, Drawable* except, Vec3 at_location);

	std::vector<EnvironmentMap*> env_maps;

  mat4x4 light_view, light_proj;
	Framebuffer shadow_map;

	Shader* pbr_shader;
	Shader* simple_shader;
	Shader* sky_shader;
	Shader* vsm_storage_shader;
	DrawParams draw_params;
	Sky* sky;

	mat4x4 cube_views[6];
	mat4x4 cube_proj;
};

}
