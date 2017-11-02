#include <stdio.h>
#include <getopt.h>
#include "core.h"
#include "body.hpp"
#include "form.hpp"
#include "camera.hpp"
#include "world.hpp"
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

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);

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
	GLFWwindow* win = init_glfw();

	const char* attrs[] = {
		"position", "normal", NULL
	};
	GLint prog = program(
		load_shader("data/basic.vsh", GL_VERTEX_SHADER),
		load_shader("data/basic.fsh", GL_FRAGMENT_SHADER),
		attrs
	);

	// int fd = open("./untitled.obj", O_RDONLY);
	// printf("%d\n", errno);
	// botshop::OBJModel mod(fd);

	botshop::World world;

	botshop::Model* car_model = botshop::ModelFactory::get_model("data/car_body.obj");
	botshop::Model* box_model = botshop::ModelFactory::get_model("data/untitled.obj");
	botshop::Form car_body(world, car_model);
	botshop::Form box1(world, box_model);
	botshop::Form box2(world, box_model);


	botshop::Camera cam(world, M_PI / 4, 160, 120);

	// Vec3 car_dims = car_model->box_dimensions();
	// printf("%f %f %f\n", car_dims.x, car_dims.y, car_dims.z);

	car_body.is_a_mesh(car_model)
	 ->position(0, 0, -1)
	 ->add_all();

	 box1.is_a_box(1, 1, 1)
 	 ->position(0, 0, 10)
 	 ->add_all();

	 box2.is_a_box(1, 1, 1)
 	 ->position(10, 0, 0)
 	 ->add_all();

	cam.is_a_sphere(0.05)
		->position(0, 0, 0)
		->add_all();

	glUseProgram(prog);


	mat4x4 proj, view;
	GLint world_uniform = glGetUniformLocation(prog, "world");
	GLint norm_uniform  = glGetUniformLocation(prog, "normal_matrix");

	GLint v_uniform     = glGetUniformLocation(prog, "view");
	GLint p_uniform     = glGetUniformLocation(prog, "projection");


	// static const GLfloat g_vertex_buffer_data[] = {
	// 	-1.0f, -1.0f, 0.0f,
	// 	 0, 0, 0,
	// 	 1.0f, -1.0f, 0.0f,
	// 	 0, 0, 0,
	// 	 0.0f,  1.0f, 0.0f,
	// 	 0, 0, 0,
	// };
	//
	// GLuint vertexbuffer;
	// glGenBuffers(1, &vertexbuffer);
	// glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

 // 	cam.torque(0, 2, 0);

 	car_body.torque(5, 5, 0);
	//cam.force(1, 0, 0);

	while(!glfwWindowShouldClose(win))
	{
		world.step(0.05);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cam.projection(proj)->view(view);
		glUniformMatrix4fv(v_uniform, 1, GL_FALSE, (GLfloat*)view);
		glUniformMatrix4fv(p_uniform, 1, GL_FALSE, (GLfloat*)proj);

		car_body.draw(world_uniform, norm_uniform);
		box1.draw(world_uniform, norm_uniform);
		box2.draw(world_uniform, norm_uniform);

		glfwPollEvents();
		glfwSwapBuffers(win);
	}

	return 0;
}
