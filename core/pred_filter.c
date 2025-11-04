#include "pred_filter.h"
#include <string.h>

bool is_type(File *f,void *ctx){
	int k=*(int*)ctx;return f->type==k;
}
bool size_gt(File *f,void *ctx){
	size_t z=*(size_t*)ctx;return f->size>z;
}
bool has_name(File *f,void *ctx){
	char *name=(char*)ctx;
	return strstr(f->name,name)!=NULL;
}
bool all_of(File *f,pred *prds,int n){
	for(int i=0;i<n;i++){
		if(!prds[i].fn(f,prds[i].ctx)){
			return false;
		}
	}
	return true;
}

