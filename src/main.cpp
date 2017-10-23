#include <stdio.h>
#include <getopt.h>
#include "core.h"
#include "body.hpp"
#include "form.hpp"
#include "camera.hpp"
#include "cli.h"
#include "ctx.h"

botshop::Context CTX;

GLFWwindow* init_glfw()
{
	if (!glfwInit()){
		exit(-1);
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* win = glfwCreateWindow(640, 480, "botshop", NULL, NULL);

	if (!win){
		glfwTerminate();
		exit(-2);
	}

	glfwMakeContextCurrent(win);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	return win;
}


GLint load_shader(const char* path, GLenum type)
{
	GLuint shader;
	GLint status = GL_TRUE;
	GLchar *source;

	int fd = open(path, O_RDONLY);

	if (fd < 0)
	{
		std::cerr << "Failed to load vertex shader '" << path << "' " << errno << std::endl;
		exit(-1);
	}

	// Load the shader source code
	size_t total_size = lseek(fd, 0, SEEK_END);
	source = (GLchar*)calloc(total_size, 1);
	lseek(fd, 0, SEEK_SET);
	read(fd, source, total_size);

	// Create the GL shader and attempt to compile it
	shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	// Print the compilation log if there's anything in there
	GLint log_length;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0)
	{
		GLchar *log_str = (GLchar *)malloc(log_length);
		glGetShaderInfoLog(shader, log_length, &log_length, log_str);
		std::cerr << "Shader compile log for '" <<  path << "' " << log_length << std::endl << log_str << std::endl;
		write(1, log_str, log_length);
		free(log_str);
	}

	// Check the status and exit on failure
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		std::cerr << "Compiling '" << path << "' failed: " << status << std::endl;
		glDeleteShader(shader);
		exit(-2);
	}

	free(source);

	return shader;
}


GLint program(GLint vertex, GLint frag, const char** attributes)
{
	GLint status;
	GLint logLength;
	GLint prog = glCreateProgram();

	glAttachShader(prog, vertex);
	glAttachShader(prog, frag);

	const char** attr = attributes;
	for(int i = 0; *attr; ++i)
	{
		glBindAttribLocation(prog, i, *attr);
		++attr;
	}

	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status == 0)
	{
		exit(-1);
	}

	glDetachShader(prog, vertex);
	glDetachShader(prog, frag);
	glDeleteShader(vertex);
	glDeleteShader(frag);

	return prog;
}


int main(int argc, char* argv[])
{
	char buf[1024];
	getcwd(buf, 1024);
	printf("%s\n", buf);
	GLFWwindow* win = init_glfw();
	getcwd(buf, 1024);
	printf("%s\n", buf);

	const char* attrs[] = {
		"position", "normal", NULL
	};
	GLint prog = program(
		load_shader("data/basic.vsh", GL_VERTEX_SHADER),
		load_shader("data/basic.fsh", GL_FRAGMENT_SHADER),
		attrs
	);

	glDisable(GL_CULL_FACE);

	// int fd = open("./untitled.obj", O_RDONLY);
	// printf("%d\n", errno);
	// botshop::OBJModel mod(fd);

	dInitODE2(0);
	dWorldID world = dWorldCreate();
	dSpaceID space = dHashSpaceCreate(0);

	botshop::Form box(world, space, botshop::ModelFactory::get_model("data/untitled.obj"));
	botshop::Camera cam(world, space, M_PI / 2, 160, 120);

	box.is_a_box(1, 1, 1)
	 ->position(2, 0, 0)
	 ->add_all();

	cam.is_a_sphere(0.05)
		->position(0, 0, 0)
		->add_all();

	dWorldSetGravity (world,0,0,-0.5);

	glUseProgram(prog);


	mat4x4 vp;
	GLint world_uniform = glGetUniformLocation(prog, "world");
	GLint vp_uniform = glGetUniformLocation(prog, "view_projection");

	while(!glfwWindowShouldClose(win))
	{
		dWorldStep(world, 0.05);

		glClear(GL_COLOR_BUFFER_BIT);

		cam.view_projection(vp);
		glUniformMatrix4fv(vp_uniform, 1, GL_FALSE, (GLfloat*)vp);

		box.draw(world_uniform);

		glfwPollEvents();
		glfwSwapBuffers(win);
	}

	return 0;
}
