#include "cli.h"

using namespace botshop;

Cli::Cli(CliOption** opts)
{
	options = opts;

	CliOption** opt = opts;
	while(*opt)
	{
		opt_count++;
		opt++;
	}
}


int Cli::process(char* const* argv, int argc, Context& ctx)
{
	struct option long_opts[opt_count];
	char short_opts[opt_count << 1];
	char* short_opt = short_opts;

	// assemble long and short arg lists
	for(int i = 0; i < opt_count; ++i)
	{
		struct option opt = {};
		opt.name = options[i]->name;
		opt.has_arg = options[i]->has_arg ? required_argument : no_argument;
		opt.val = options[i]->flag;

		long_opts[i] = opt;
		*short_opt = opt.val;
		++short_opt;
		if(options[i]->has_arg)
		{
			*short_opt = ':';
			++short_opt;
		}
	}

	// Process all args
	int ch;
	while((ch = getopt_long(argc, argv, short_opts, long_opts, (int*)0)) != -1)
	{
		for(int i = opt_count; i--;)
		{
			CliOption opt = *options[i];

			if(ch == opt.flag)
			{
				opt.apply(ctx, optarg);
			}
		}
	}

	return 0;
}
