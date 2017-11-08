#include "material.hpp"
#include <png.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif

using namespace botshop;


Framebuffer MaterialFactory::create_framebuffer(int width, int height)
{
	Framebuffer fbo;

	glGenFramebuffers(1, &fbo.id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);

	fbo.color = create_texture(width, height, GL_RGB, NULL);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		fbo.color,
		0
	);

	glGenRenderbuffers(1, &fbo.depth);
	glBindRenderbuffer(GL_RENDERBUFFER, fbo.depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo.depth);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	return fbo;
}
//------------------------------------------------------------------------------

GLuint MaterialFactory::create_texture(int width, int height, GLenum format, void* data)
{
	GLuint tex;

	assert(gl_get_error());
	glGenTextures(1, &tex);
	assert(gl_get_error());

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format,
		width, height,
		0,
		format,
		GL_UNSIGNED_BYTE,
		(void*)data
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	return tex;
}
//------------------------------------------------------------------------------

void abort(std::string message)
{
	std::cerr << message << std::endl;
	exit(-1);
}
//------------------------------------------------------------------------------

void Material::use(GLint* material_uniforms)
{
	static long bound;

	long mat_id = v[0] + v[1] * 10 + v[2] * 100;

	if(bound == mat_id) return;

	for(int i = 3; i--;)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, v[i]);
		glUniform1i(material_uniforms[i], i);
	}

	bound = mat_id;
}
//------------------------------------------------------------------------------

GLuint MaterialFactory::load_texture(std::string path)
{
	char header[8];    // 8 is the maximum size that can be checked
	png_structp png_ptr = {};
	png_infop info_ptr;
	png_bytep* row_pointers;
	png_byte color_type;
	png_byte bit_depth;
	int width, height;
	int number_of_passes;
	GLuint tex;
	GLenum gl_color_type;

	std::cerr << "loading texture '" <<  path << "'... ";

	/* open file and test for it being a png */
	FILE *fp = fopen(path.c_str(), "rb");
	if (!fp)
	{
		fprintf(stderr, "[read_png_file] File %s could not be opened for reading", path.c_str());
	}

	fread(header, 1, 8, fp);
	if (png_sig_cmp((png_const_bytep)header, 0, 8))
	{
		fprintf(stderr, "[read_png_file] File %s is not recognized as a PNG file", path.c_str());
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

	int color_components;
	switch (color_type) {
		case PNG_COLOR_TYPE_RGBA:
			gl_color_type = GL_RGBA;
			color_components = 4;
			break;
		case PNG_COLOR_TYPE_PALETTE:
		case PNG_COLOR_TYPE_RGB:
			gl_color_type = GL_RGB;
			color_components = 3;
			break;
	}

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	char pixel_buf[color_components * width * height];

	for (int y = 0; y < height; y++)
	{
		row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
		assert(row_pointers[y]);
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

	tex = create_texture(width, height, gl_color_type, pixel_buf);

	assert(gl_get_error());

	std::cerr << "OK" << std::endl;

	return tex;
}
//------------------------------------------------------------------------------

Material* MaterialFactory::get_material(const std::string path)
{
	static std::map<std::string, Material*> _cached_materials;

	if(_cached_materials.count(path) == 0)
	{
		Material* material = new Material();
		material->textures.color    = load_texture(path + ".color.png");
		material->textures.normal   = load_texture(path + ".normal.png");
		material->textures.specular = load_texture(path + ".specular.png");

		_cached_materials[path] = material;
	}

	return _cached_materials[path];
}
