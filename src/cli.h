#pragma once

#include <getopt.h>
#include "ctx.h"

namespace botshop {

struct CliOption {
	const char* name;
	const char* desc;
	char flag;
	int has_arg;
	void (*apply)(Context& ctx, char* value);
};

class Cli {
public:
	Cli(CliOption** options);
	~Cli();

	int process(char* const* argv, int argc, Context& ctx);

private:
	CliOption** options;
	unsigned int opt_count;
};

}
