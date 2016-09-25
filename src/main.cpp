#include <stdio.h>
#include <opt.h>
#include "core.h"

int main(int argc, char* argv[])
{
	USE_OPT
	OPT_LIST_START

	OPT_LIST_END(BS_TITLE);

	dWorldCreate();

	return 0;
}
