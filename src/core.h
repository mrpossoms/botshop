#pragma once

// system libs
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

// c++ libs
#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <streambuf>

// project libs
#ifdef __linux__
#include <GL/glx.h>
#include <GL/glext.h>
#else
#include <OpenGL/gl3.h>
#endif
#include <GLFW/glfw3.h>
// #include <GL/glew.h>
#include <ode/ode.h>
#include <linmath.h>
#include "constants.h"
#include "interfaces.hpp"

namespace botshop {

static char* str_from_file(const char* path)
{
	char* str = NULL;

	int fd = open(path, O_RDONLY);
	assert(fd);

	size_t file_size = lseek(fd, SEEK_END, 0);
	str = (char*)malloc(file_size);
	assert(str);

	assert(read(fd, str, file_size) == file_size);
	close(fd);

	return str;
}

}
