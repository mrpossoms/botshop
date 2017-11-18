#include "renderergl.hpp"
#include "camera.hpp"
#include "material.hpp"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif

using namespace botshop;


EnvironmentMap::EnvironmentMap(int res)
{
	size = res;

	GLenum sides[] = {
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	};

	glGenTextures(1, &map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, map);

	assert(gl_get_error());

	for(int i = 6; i--;)
	{
		glTexImage2D(sides[i], 0, GL_RGB, res, res, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}

	assert(gl_get_error());

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	assert(gl_get_error());

	framebuffer = MaterialFactory::create_framebuffer(res, res, Framebuffer::depth_flag);
}
//------------------------------------------------------------------------------

void EnvironmentMap::render_to(GLenum face)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, map);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, face, map, 0);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}
//------------------------------------------------------------------------------

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
//------------------------------------------------------------------------------

static GLint link_program(GLint vertex, GLint frag, const char** attributes)
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
//------------------------------------------------------------------------------

Shader::Shader(GLint vertex, GLint frag)
{
	const char* attrs[] = {
		"position", "normal", "tangent", "texcoord", NULL
	};

	program = link_program(vertex, frag, attrs);

	draw_params.world_uniform = glGetUniformLocation(program, "world_matrix");
	draw_params.norm_uniform  = glGetUniformLocation(program, "normal_matrix");
	draw_params.view_uniform  = glGetUniformLocation(program, "view_matrix");
	draw_params.proj_uniform  = glGetUniformLocation(program, "proj_matrix");

	draw_params.material_uniforms.tex  = glGetUniformLocation(program, "tex");
	draw_params.material_uniforms.norm = glGetUniformLocation(program, "norm");
	draw_params.material_uniforms.spec = glGetUniformLocation(program, "spec");
	draw_params.material_uniforms.envd = glGetUniformLocation(program, "envd");
}

//------------------------------------------------------------------------------
static GLFWwindow* init_glfw(int width, int height)
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

	GLFWwindow* win = glfwCreateWindow(width, height, "botshop", NULL, NULL);

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

	assert(gl_get_error());

	return win;
}
//------------------------------------------------------------------------------

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
//------------------------------------------------------------------------------

RendererGL::RendererGL(std::string data_path)
{
	width = 640;
	height = 480;

	win = init_glfw(width, height);

	assert(gl_get_error());

	simple_shader = new Shader(
		load_shader("data/basic.vsh", GL_VERTEX_SHADER),
		load_shader("data/basic.fsh", GL_FRAGMENT_SHADER)
	);

	pbr_shader = new Shader(
		load_shader("data/basic.vsh", GL_VERTEX_SHADER),
		load_shader("data/global.fsh", GL_FRAGMENT_SHADER)
	);

	Material* brick_material = MaterialFactory::get_material("data/brick");
	GLuint BRDF_LUT = MaterialFactory::load_texture("data/brdfLUT.png");

	env = new EnvironmentMap(64);

	for(int i = 10; i--;)
	{
		env_maps.push_back(new EnvironmentMap(64));
	}

	glUseProgram(pbr_shader->program);

	mat4x4 proj, view;

	GLint material_uniform = glGetUniformLocation(pbr_shader->program, "material");
	GLint albedo_uniform = glGetUniformLocation(pbr_shader->program, "albedo");

	vec4 material = { 0.1, 0.01, 1, 0.01 };
	vec4 albedo = { 1, 1, 1, 1 };

	glUniform4fv(material_uniform, 1, (GLfloat*)material);
	glUniform4fv(albedo_uniform, 1, (GLfloat*)albedo);

	brick_material->use(&pbr_shader->draw_params.material_uniforms.tex);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, BRDF_LUT);
	glUniform1i(glGetUniformLocation(pbr_shader->program, "iblbrdf"), 3);

	assert(gl_get_error());

}
//------------------------------------------------------------------------------

void RendererGL::draw_scene(Scene* scene)
{
	assert(gl_get_error());

	scene->draw(&draw_params);

	assert(gl_get_error());

	for(auto drawable : scene->drawables())
	{
		drawable->draw(&draw_params);
	}

	assert(gl_get_error());

}
//------------------------------------------------------------------------------

void RendererGL::draw_to(EnvironmentMap* env, Scene* scene, Drawable* except, Vec3 at_location)
{
	static bool setup;

	struct basis {
		Vec3 up, forward;
	};

	if(!setup)
	{

		basis bases[] = {
			{ VEC3_UP,      VEC3_BACK    },
			{ VEC3_UP,      VEC3_FORWARD },
			{ VEC3_FORWARD, VEC3_UP      },
			{ VEC3_BACK,    VEC3_DOWN    },
			{ VEC3_DOWN,    VEC3_RIGHT   },
			{ VEC3_DOWN,    VEC3_LEFT    },
		};
		mat4x4_perspective(cube_proj, M_PI / 2, 1, 0.01, 1000);

		for(int i = 6; i--;)
		{
			Vec3 eye = at_location + bases[i].forward;

			mat4x4_look_at(
				cube_views[i],
				at_location.v,
				eye.v,
				bases[i].up.v
			);
		}

		setup = true;
	}

	GLenum sides[] = {
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	};

	glUniformMatrix4fv(draw_params.proj_uniform, 1, GL_FALSE, (GLfloat*)cube_proj);
	glViewport(0, 0, env->size, env->size);

	assert(gl_get_error());

	int i = 0;
	for(; i < 6; i++)
	{
		env->render_to(sides[i]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniformMatrix4fv(draw_params.view_uniform, 1, GL_FALSE, (GLfloat*)cube_views[i]);

		assert(gl_get_error());

		scene->draw(&draw_params);

		assert(gl_get_error());

		for(auto drawable : scene->drawables())
		{
			if(drawable == except) continue;
			drawable->draw(&draw_params);
		}

		assert(gl_get_error());
	}

	assert(gl_get_error());
}
//------------------------------------------------------------------------------

void RendererGL::draw(Viewer* viewer, Scene* scene)
{
	assert(gl_get_error());

	mat4x4 view, proj;



	assert(gl_get_error());

	draw_params = simple_shader->draw_params;
	glUseProgram(simple_shader->program);

	glClearColor(135 / 255.f, 206 / 255.f, 235 / 255.f, 1);

	int env_idx = 0;
	for(auto drawable : scene->drawables())
	{
		Dynamic* body = dynamic_cast<Dynamic*>(drawable);
		if(!body) continue;

		Vec3 pos = body->position();
		draw_to(env_maps[env_idx], scene, drawable, pos);
		++env_idx;
	}

	// Set the screen's framebuffer as the render target
	// get ready to use the PBR shader
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	draw_params = pbr_shader->draw_params;
	glUseProgram(pbr_shader->program);

	if(glfwGetKey(win, GLFW_KEY_F))
	{
		draw_params = simple_shader->draw_params;
		glUseProgram(simple_shader->program);
	}

	// Make the sky black, and clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int fb_width, fb_height;
	glfwGetFramebufferSize(win, &fb_width, &fb_height);
	glViewport(0, 0, fb_width, fb_height);

	int side_view_keys[] = {
		GLFW_KEY_1,
		GLFW_KEY_2,
		GLFW_KEY_3,
		GLFW_KEY_4,
		GLFW_KEY_5,
		GLFW_KEY_6,
	};

	viewer->projection(proj)->view(view);

	for(int i = 6; i--;)
	{
		if(glfwGetKey(win, side_view_keys[i]))
		{
			mat4x4_dup(view, cube_views[i]);
			mat4x4_dup(proj, cube_proj);
		}
	}

	glUniformMatrix4fv(draw_params.view_uniform, 1, GL_FALSE, (GLfloat*)view);
	glUniformMatrix4fv(draw_params.proj_uniform, 1, GL_FALSE, (GLfloat*)proj);

	assert(gl_get_error());

	env_idx = 0;
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_maps[env_idx]->map);
	glUniform1i(draw_params.material_uniforms.envd, 4);

	scene->draw(&draw_params);

	assert(gl_get_error());

	for(auto drawable : scene->drawables())
	{
		// Bind the cubemap rendered earlier to use for IBL
		glBindTexture(GL_TEXTURE_CUBE_MAP, env_maps[env_idx++]->map);
		glUniform1i(draw_params.material_uniforms.envd, 4);

		drawable->draw(&draw_params);
	}

	assert(gl_get_error());

	glfwPollEvents();
	glfwSwapBuffers(win);

	Camera* cam = dynamic_cast<Camera*>(viewer);
	if(cam) free_fly(win, cam);
}
