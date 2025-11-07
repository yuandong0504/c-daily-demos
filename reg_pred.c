#include <stdio.h>
#include <stdbool.h>
#include <string.h>
typedef struct {
	int type;size_t z;const char* name;
}File;
typedef bool (*predicate_fn)(File *f,void *ctx);
bool is_type(File *f,void *cxt){
	int k=*(int*)cxt;return f->type==k;
}
bool size_gt(File *f,void *ctx){
	size_t k=*(size_t*)ctx;return f->z>k;
}
bool has_name(File *f,void *ctx){
	char *s=(char*)ctx;
	return strstr(f->name,s)!=NULL;
}
typedef struct {
	const char *name;predicate_fn fn;
}PredEntry;
static PredEntry REG[16];
static int REG_N=0;
int pred_register(char *name,predicate_fn fn){
	if(REG_N>=sizeof(REG)/sizeof(REG[0]))return -1;
	for(int i=0;i<REG_N;i++){
		if(strcmp(REG[i].name,name)==0){
			REG[i].fn=fn;return 0;
		}
	}
	REG[REG_N++]=(PredEntry){name,fn};
	return 0;
}
predicate_fn pred_get(const char *name){
	for(int i=0;i<REG_N;i++){
		if(strcmp(REG[i].name,name)==0){
			return REG[i].fn;
		}
	}
	return NULL;
}
int main(){
	pred_register("is_type",is_type);
	pred_register("size_gt",size_gt);
	pred_register("has_name",has_name);
	for(int i=0;i<REG_N;i++){
		printf("%s\n",REG[i].name);
	}
}
