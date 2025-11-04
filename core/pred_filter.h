#pragma once
#include "model.h"
#include <stdbool.h>

typedef struct{
	bool (*fn)(File *f,void *ctx);
	void *ctx;
}pred;
bool is_type(File *f,void *ctx);
bool size_gt(File *f,void *ctx);
bool has_name(File *f,void *ctx);
bool all_of(File *f,pred *prds,int n);
