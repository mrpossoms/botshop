#include <stdio.h>
#include <getopt.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif
#include "core.h"
#include "body.hpp"
#include "form.hpp"
#include "camera.hpp"
#include "world.hpp"
#include "material.hpp"
#include "cli.h"
#include "ctx.h"

botshop::Context CTX;

GLFWwindow* init_glfw()
{
	if (!glfwInit()){
		fprintf(stderr, "glfwInit() failed\n");
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
		fprintf(stderr, "glfwCreateWindow() failed\n");
		exit(-2);
	}

	glfwMakeContextCurrent(win);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_TEXTURE_2D);

	assert(botshop::gl_get_error());

	return win;
}


GLint load_shader(const char* path, GLenum type)
{
	GLuint shader;
	GLint status = GL_TRUE;
	GLchar *source;

	int fd = open(path, O_RDONLY);

	std::cerr << "compiling '" << path << "'... ";

	if (fd < 0)
	{
		fprintf(stderr, "Failed to load vertex shader '%s' %d\n", path, errno);
	}

	// Load the shader source code
	size_t total_size = lseek(fd, 0, SEEK_END);
	source = (GLchar*)calloc(total_size, 1);
	lseek(fd, 0, SEEK_SET);
	read(fd, source, total_size);

	assert(botshop::gl_get_error());

	// Create the GL shader and attempt to compile it
	shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	assert(botshop::gl_get_error());

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

	assert(botshop::gl_get_error());

	// Check the status and exit on failure
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		std::cerr << "Compiling '" << path << "' failed: " << status << std::endl;
		glDeleteShader(shader);
		exit(-2);
	}

	assert(botshop::gl_get_error());

	free(source);

	std::cerr << "ok" << std::endl;

	return shader;
}


GLint program(GLint vertex, GLint frag, const char** attributes)
{
	GLint status;
	GLint logLength;
	GLint prog = glCreateProgram();

	assert(botshop::gl_get_error());

	glAttachShader(prog, vertex);
	glAttachShader(prog, frag);

	assert(botshop::gl_get_error());

	const char** attr = attributes;
	for(int i = 0; *attr; ++i)
	{
		glBindAttribLocation(prog, i, *attr);
		++attr;
	}

	assert(botshop::gl_get_error());

	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status == 0)
	{
		GLint log_length;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &log_length);
		if (log_length > 0)
		{
			GLchar *log_str = (GLchar *)malloc(log_length);
			glGetProgramInfoLog(prog, log_length, &log_length, log_str);
			std::cerr << "Shader link log: " << log_length << std::endl << log_str << std::endl;
			write(1, log_str, log_length);
			free(log_str);
		}
		exit(-1);
	}

	assert(botshop::gl_get_error());

	glDetachShader(prog, vertex);
	glDetachShader(prog, frag);
	glDeleteShader(vertex);
	glDeleteShader(frag);

	return prog;
}


int main(int argc, char* argv[])
{
	GLFWwindow* win = init_glfw();

	assert(botshop::gl_get_error());

	const char* attrs[] = {
		"position", "normal", "tangent", "texcoord", NULL
	};
	GLint prog = program(
		load_shader("data/basic.vsh", GL_VERTEX_SHADER),
		load_shader("data/pbr.fsh", GL_FRAGMENT_SHADER),
		// load_shader("data/basic.fsh", GL_FRAGMENT_SHADER),
		attrs
	);

	srand(time(NULL));

	assert(botshop::gl_get_error());

	// int fd = open("./untitled.obj", O_RDONLY);
	// printf("%d\n", errno);
	// botshop::OBJModel mod(fd);

	botshop::World world;

	botshop::Model* car_model = botshop::ModelFactory::get_model("data/car_body.obj");
	botshop::Model* wheel_model = botshop::ModelFactory::get_model("data/wheel.obj");
	botshop::Model* box_model = botshop::ModelFactory::get_model("data/untitled.obj");

	botshop::Form car_body(world, car_model);
	botshop::Form box0(world, box_model);
	botshop::Form box1(world, box_model);
	botshop::Form box2(world, box_model);
	botshop::Form wheel0(world, wheel_model);
	botshop::Form wheel1(world, wheel_model);
	botshop::Form wheel2(world, wheel_model);
	botshop::Form wheel3(world, wheel_model);

	botshop::Material* brick_material = botshop::MaterialFactory::get_material("data/brick");

	botshop::Camera cam(world, M_PI / 8, 160, 120);

	Vec3 cd = car_model->box_dimensions();
	printf("%f %f %f\n", cd.x, cd.y, cd.z);

	// car_body.is_a_mesh(car_model)->position(0, 0, 9);
	car_body.is_a_box(cd.x, cd.y, cd.z)->position(0, 0, 1);
	wheel0.is_a_sphere(0.082)->position(0.15, 0.15, 1);
	wheel1.is_a_sphere(0.082)->position(-0.15, 0.15, 1);
	wheel2.is_a_sphere(0.082)->position(-0.15, -0.15, 1);
	wheel3.is_a_sphere(0.082)->position(0.15, -0.15, 1);
	car_body + botshop::Joint::wheel(wheel0, Vec3(0, 0, 1), Vec3(1, 0, 0));
	car_body + botshop::Joint::wheel(wheel1, Vec3(0, 0, 1), Vec3(1, 0, 0));
	botshop::Joint* rear_wheel_axle0 = botshop::Joint::wheel(wheel2, Vec3(0, 0, 1), Vec3(1, 0, 0));
	botshop::Joint* rear_wheel_axle1 = botshop::Joint::wheel(wheel3, Vec3(0, 0, 1), Vec3(1, 0, 0));

	car_body.attach(rear_wheel_axle0);

	box0.is_a_box(2, 2, 2)->position(0, 2, 5);
	box1.is_a_box(2, 2, 2)->position(-3, 0, 10);
	box2.is_a_box(2, 2, 2)->position(3, 0, 10);

	cam.is_a_sphere(0.05)->position(0, 0, 10);

	glUseProgram(prog);

	mat4x4 proj, view;
	GLint world_uniform = glGetUniformLocation(prog, "world_matrix");
	GLint norm_uniform  = glGetUniformLocation(prog, "normal_matrix");

	GLint v_uniform     = glGetUniformLocation(prog, "view_matrix");
	GLint p_uniform     = glGetUniformLocation(prog, "proj_matrix");

	GLint material_uniform = glGetUniformLocation(prog, "material");
	GLint albedo_uniform = glGetUniformLocation(prog, "albedo");

	GLint material_uniforms[] = {
		glGetUniformLocation(prog, "tex"),
		glGetUniformLocation(prog, "norm"),
		glGetUniformLocation(prog, "spec")
	};

	assert(botshop::gl_get_error());

	world += car_body;
	world += wheel0;
	world += wheel1;
	world += wheel2;
	world += wheel3;
	// world += box0;
	world += box1;
	world += box2;
	world += cam;

 // 	cam.torque(1, 0, 1);

 	car_body.torque(botshop::randf(20) - 10, botshop::randf(20) - 10, botshop::randf(20) - 10);

	box0.torque(botshop::randf(20) - 10, botshop::randf(20) - 10, botshop::randf(20) - 10);
	box1.torque(botshop::randf(20) - 10, botshop::randf(20) - 10, botshop::randf(20) - 10);
	box2.torque(botshop::randf(20) - 10, botshop::randf(20) - 10, botshop::randf(20) - 10);

	botshop::DrawParams draw_params = {
		.world_uniform = world_uniform,
		.norm_uniform  = norm_uniform,
	};

	vec4 material = { 0.1, 0.1, 1, 0.01 };
	vec4 albedo = { 1, 1, 1, 1 };

	glUniform4fv(material_uniform, 1, (GLfloat*)material);
	glUniform4fv(albedo_uniform, 1, (GLfloat*)albedo);


	brick_material->use(material_uniforms);

	assert(botshop::gl_get_error());

	time_t now = time(NULL);
	int frames = 0;

	while(!glfwWindowShouldClose(win))
	{
		dJointSetHinge2Param (rear_wheel_axle0->ode_joint,dParamVel2,-0.1);
	    dJointSetHinge2Param (rear_wheel_axle0->ode_joint,dParamFMax2,0.1);
		dJointSetHinge2Param (rear_wheel_axle1->ode_joint,dParamVel2,-0.1);
	    dJointSetHinge2Param (rear_wheel_axle1->ode_joint,dParamFMax2,0.1);

		world.step(0.05);
		cam.position(0, 0, 10);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cam.projection(proj)->view(view);
		glUniformMatrix4fv(v_uniform, 1, GL_FALSE, (GLfloat*)view);
		glUniformMatrix4fv(p_uniform, 1, GL_FALSE, (GLfloat*)proj);

		world.draw(&draw_params);

		glfwPollEvents();
		glfwSwapBuffers(win);

		if(now != time(NULL))
		{
			now = time(NULL);
			std::cerr << frames << "fps\n";
			frames = 0;
		}

		++frames;
	}

	return 0;
}
