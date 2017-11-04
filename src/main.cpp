#include <stdio.h>
#include <getopt.h>
#include <png.h>
#include "core.h"
#include "body.hpp"
#include "form.hpp"
#include "camera.hpp"
#include "world.hpp"
#include "cli.h"
#include "ctx.h"

botshop::Context CTX;

void abort(std::string message)
{
	std::cerr << message << std::endl;
	exit(-1);
}

float randf(float f)
{
	return f * (random() % 2048) / 2048.f;
}

bool gl_get_error()
{
	GLenum err = GL_NO_ERROR;
	bool good = true;

	while((err = glGetError()) != GL_NO_ERROR)
	{
		std::cerr << "GL_ERROR: 0x" << std::hex << err << std::endl;
		good = false;
	}

	return good;
}

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

	assert(gl_get_error());

	return win;
}


GLuint load_texture(const char* path)
{
	char header[8];    // 8 is the maximum size that can be checked
	png_structp png_ptr = {};
	png_infop info_ptr;
	png_bytep* row_pointers;
	png_byte color_type;
	png_byte bit_depth;
	int width, height;
	int number_of_passes;

	/* open file and test for it being a png */
	FILE *fp = fopen(path, "rb");
	if (!fp)
	{
		fprintf(stderr, "[read_png_file] File %s could not be opened for reading", path);
	}

	fread(header, 1, 8, fp);
	if (png_sig_cmp((png_const_bytep)header, 0, 8))
	{
		fprintf(stderr, "[read_png_file] File %s is not recognized as a PNG file", path);
	}


	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort("[read_png_file] png_create_read_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort("[read_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort("[read_png_file] Error during init_io");

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);


	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		abort("[read_png_file] Error during read_image");
	}

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	char pixel_buf[3 * width * height];

	for (int y = 0; y < height; y++)
	{
		row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
	}

	png_read_image(png_ptr, row_pointers);

	int bytes_per_row = png_get_rowbytes(png_ptr,info_ptr);
	for (int y = 0; y < height; y++)
	{
		memcpy(pixel_buf + (y * bytes_per_row), row_pointers[y], bytes_per_row);
		free(row_pointers[y]);
	}
	free(row_pointers);

	fclose(fp);

	GLuint tex;
	GLenum gl_color_type;

	switch (color_type) {
		case PNG_COLOR_TYPE_RGBA:
			gl_color_type = GL_RGBA;
			break;
		case PNG_COLOR_TYPE_PALETTE:
		case PNG_COLOR_TYPE_RGB:
			gl_color_type = GL_RGB;
			break;
	}

	// int fd = open("/dev/random", O_RDONLY);
	// read(fd, pixel_buf, 3 * width * height);
	// close(fd);

	assert(gl_get_error());
	glGenTextures(1, &tex);
	assert(gl_get_error());

	// memset(pixel_buf, 64, sizeof(pixel_buf));

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		gl_color_type,
		width, height,
		0,
		gl_color_type,
		GL_UNSIGNED_BYTE,
		(void*)pixel_buf
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	assert(gl_get_error());

	return tex;
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


GLint program(GLint vertex, GLint frag, const char** attributes)
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


int main(int argc, char* argv[])
{
	GLFWwindow* win = init_glfw();

	assert(gl_get_error());

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

	assert(gl_get_error());

	// int fd = open("./untitled.obj", O_RDONLY);
	// printf("%d\n", errno);
	// botshop::OBJModel mod(fd);

	botshop::World world;

	botshop::Model* car_model = botshop::ModelFactory::get_model("data/car_body.obj");
	botshop::Model* box_model = botshop::ModelFactory::get_model("data/untitled.obj");
	botshop::Form car_body(world, car_model);
	botshop::Form box0(world, box_model);
	botshop::Form box1(world, box_model);
	botshop::Form box2(world, box_model);

	GLuint test_tex = load_texture("data/color.png");
	GLuint test_norm = load_texture("data/normal.png");
	GLuint test_spec = load_texture("data/specular.png");

	printf("test_tex %d\n", test_tex);

	botshop::Camera cam(world, M_PI / 4, 160, 120);

	Vec3 cd = car_model->box_dimensions();
	// printf("%f %f %f\n", car_dims.x, car_dims.y, car_dims.z);

	// car_body.is_a_mesh(car_model)->position(0, 0, 9);
	car_body.is_a_box(cd.x, cd.y, cd.z)->position(0, 0, 9);

	box0.is_a_box(2, 2, 2)->position(0, 0, 5);
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

	GLint texture_uniform = glGetUniformLocation(prog, "tex");
	GLint normal_uniform = glGetUniformLocation(prog, "norm");
	GLint specular_uniform = glGetUniformLocation(prog, "spec");

	assert(gl_get_error());

	world += car_body;
	world += box0;
	world += box1;
	world += box2;
	world += cam;

 // 	cam.torque(1, 0, 1);

 	car_body.torque(randf(20) - 10, randf(20) - 10, randf(20) - 10);
	//cam.force(1, 0, 0);

	box0.torque(randf(20) - 10, randf(20) - 10, randf(20) - 10);
	box1.torque(randf(20) - 10, randf(20) - 10, randf(20) - 10);
	box2.torque(randf(20) - 10, randf(20) - 10, randf(20) - 10);

	botshop::DrawParams draw_params = {
		.world_uniform = world_uniform,
		.norm_uniform  = norm_uniform,
	};

	vec4 material = { 0.1, 0.1, 1, 0.01 };
	vec4 albedo = { 1, 1, 1, 1 };

	glUniform4fv(material_uniform, 1, (GLfloat*)material);
	glUniform4fv(albedo_uniform, 1, (GLfloat*)albedo);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, test_tex);
	glUniform1i(texture_uniform, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, test_spec);
	glUniform1i(specular_uniform, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, test_norm);
	glUniform1i(normal_uniform, 2);

	assert(gl_get_error());

	while(!glfwWindowShouldClose(win))
	{
		world.step(0.05);
		cam.position(0, 0, 10);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cam.projection(proj)->view(view);
		glUniformMatrix4fv(v_uniform, 1, GL_FALSE, (GLfloat*)view);
		glUniformMatrix4fv(p_uniform, 1, GL_FALSE, (GLfloat*)proj);

		world.draw(&draw_params);

		glfwPollEvents();
		glfwSwapBuffers(win);
	}

	return 0;
}
