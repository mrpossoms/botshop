#pragma once

#include "core.h"

namespace botshop
{

class RendererGL : public Renderer
{
public:
	RendererGL(std::string data_path);

	virtual void draw(Viewer* viewer, Scene* scene);

	GLFWwindow* win;
private:
	GLint shader_prog;
	DrawParams draw_params;
};

}
