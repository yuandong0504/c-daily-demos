#pragma once
#include <stddef.h>
typedef struct File{
	int type;
	size_t size;
	const char *name;
}File;
