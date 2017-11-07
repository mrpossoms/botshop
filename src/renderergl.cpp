#include "renderergl.hpp"
#include "camera.hpp"
#include "material.hpp"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif

using namespace botshop;

static GLFWwindow* init_glfw()
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
	glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_TEXTURE_2D);

	assert(gl_get_error());

	return win;
}

static GLint load_shader(const char* path, GLenum type)
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

	assert(gl_get_error());

	// Create the GL shader and attempt to compile it
	shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	assert(gl_get_error());

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

	assert(gl_get_error());

	// Check the status and exit on failure
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		std::cerr << "Compiling '" << path << "' failed: " << status << std::endl;
		glDeleteShader(shader);
		exit(-2);
	}

	assert(gl_get_error());

	free(source);

	std::cerr << "ok" << std::endl;

	return shader;
}


static GLint program(GLint vertex, GLint frag, const char** attributes)
{
	GLint status;
	GLint logLength;
	GLint prog = glCreateProgram();

	assert(gl_get_error());

	glAttachShader(prog, vertex);
	glAttachShader(prog, frag);

	assert(gl_get_error());

	const char** attr = attributes;
	for(int i = 0; *attr; ++i)
	{
		glBindAttribLocation(prog, i, *attr);
		++attr;
	}

	assert(gl_get_error());

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

	assert(gl_get_error());

	glDetachShader(prog, vertex);
	glDetachShader(prog, frag);
	glDeleteShader(vertex);
	glDeleteShader(frag);

	return prog;
}

static double lx, ly;
static void free_fly(GLFWwindow* win, Camera* cam)
{
	static bool setup;
	double x = 0, y = 0;
	double dx, dy;

	glfwGetCursorPos(win, &x, &y);

	if(!setup)
	{
		lx = x;
		ly = y;
		setup = true;
	}

	dx = x - lx; dy = y - ly;

	{
		Quat q = cam->orientation();
		Quat pitch, yaw, roll;
		Vec3 forward, left, up;

		pitch.from_axis_angle(VEC3_LEFT.v[0], VEC3_LEFT.v[1], VEC3_LEFT.v[2], dy * 0.01);
		yaw.from_axis_angle(VEC3_UP.v[0], VEC3_UP.v[1], VEC3_UP.v[2], dx * 0.01);
		pitch = pitch * yaw;
		q = pitch * q;

		quat q_star;
		quat_conj(q_star, q.v);
		quat_mul_vec3(forward.v, q_star, VEC3_FORWARD.v);
		quat_mul_vec3(left.v, q_star, VEC3_LEFT.v);
		quat_mul_vec3(up.v, q_star, VEC3_UP.v);

		if(glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS)
		{
			roll.from_axis_angle(VEC3_FORWARD.x, VEC3_FORWARD.y, VEC3_FORWARD.z, -0.025);
		}
		if(glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS)
		{
			roll.from_axis_angle(VEC3_FORWARD.x, VEC3_FORWARD.y, VEC3_FORWARD.z, 0.025);
		}

		q = roll * q;
		cam->orientation(q);

		int keys[] = {
			GLFW_KEY_W,
			GLFW_KEY_S,
			GLFW_KEY_D,
			GLFW_KEY_A,
			GLFW_KEY_SPACE,
			GLFW_KEY_LEFT_SHIFT,
		};

		const float speed = 0.05;
		Vec3 dirs[] = {
			forward * -speed,
			forward *  speed,
			left    *  speed,
			left    * -speed,
			up      *  speed,
			up      * -speed,
		};

		for(int i = sizeof(keys) / sizeof(int); i--;)
		if(glfwGetKey(win, keys[i]) == GLFW_PRESS)
		{
			Vec3 pos = cam->position() + dirs[i];
			cam->position(pos);
		}
	}

	lx = x; ly = y;
}


RendererGL::RendererGL(std::string data_path)
{
	win = init_glfw();

	assert(gl_get_error());

	const char* attrs[] = {
		"position", "normal", "tangent", "texcoord", NULL
	};
	shader_prog = program(
		load_shader("data/basic.vsh", GL_VERTEX_SHADER),
		load_shader("data/pbr.fsh", GL_FRAGMENT_SHADER),
		// load_shader("data/basic.fsh", GL_FRAGMENT_SHADER),
		attrs
	);

	Material* brick_material = MaterialFactory::get_material("data/brick");
	GLuint BRDF_LUT = MaterialFactory::load_texture("data/brdfLUT.png");

	glUseProgram(shader_prog);

	mat4x4 proj, view;
	GLint world_uniform = glGetUniformLocation(shader_prog, "world_matrix");
	GLint norm_uniform  = glGetUniformLocation(shader_prog, "normal_matrix");

	GLint material_uniform = glGetUniformLocation(shader_prog, "material");
	GLint albedo_uniform = glGetUniformLocation(shader_prog, "albedo");

	GLint material_uniforms[] = {
		glGetUniformLocation(shader_prog, "tex"),
		glGetUniformLocation(shader_prog, "norm"),
		glGetUniformLocation(shader_prog, "spec")
	};

	draw_params = {
		.world_uniform = world_uniform,
		.norm_uniform  = norm_uniform,
	};

	vec4 material = { 0.1, 0.01, 1, 0.01 };
	vec4 albedo = { 1, 1, 1, 1 };

	glUniform4fv(material_uniform, 1, (GLfloat*)material);
	glUniform4fv(albedo_uniform, 1, (GLfloat*)albedo);

	brick_material->use(material_uniforms);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, BRDF_LUT);
	glUniform1i(glGetUniformLocation(shader_prog, "iblbrdf"), 4);

	assert(gl_get_error());

}

void RendererGL::draw(Viewer* viewer, Scene* scene)
{
	mat4x4 view, proj;
	GLint v_uniform     = glGetUniformLocation(shader_prog, "view_matrix");
	GLint p_uniform     = glGetUniformLocation(shader_prog, "proj_matrix");

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	viewer->projection(proj)->view(view);
	glUniformMatrix4fv(v_uniform, 1, GL_FALSE, (GLfloat*)view);
	glUniformMatrix4fv(p_uniform, 1, GL_FALSE, (GLfloat*)proj);

	scene->draw(&draw_params);

	for(auto drawable : scene->drawables())
	{
		drawable->draw(&draw_params);
	}

	glfwPollEvents();
	glfwSwapBuffers(win);

	Camera* cam = dynamic_cast<Camera*>(viewer);
	if(cam) free_fly(win, cam);
}
