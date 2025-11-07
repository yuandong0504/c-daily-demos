#include <stdio.h>
#include <string.h>
#include <stdbool.h>
typedef struct File{
	int type;size_t size;const char* name;
}File;
typedef bool (*predicate_fn)(File *f,void *ctx);
typedef struct{predicate_fn fn;void *ctx;}pred;
bool is_type(File *f,void *ctx){
	int k=*(int *)ctx;return f->type==k;
}
bool size_gt(File* f,void *ctx){
	size_t k=*(size_t*)ctx;return f->size>k;
}
bool has_name(File *f,void *ctx){
	char *name=(char*)ctx;
	return strstr(f->name,name)!=NULL;
}
struct ctx2{predicate_fn a,b;void *c1,*c2;};
bool type_size(File* f,void *ctx){
	struct ctx2 *c=ctx;
	return c->a(f,c->c1)&&c->b(f,c->c2);
}
bool all_of(File *f,pred *pds,int n){
	for(int i=0;i<n;i++){
		if(!pds[i].fn(f,pds[i].ctx)){
			return false;
		}
	}
	return true; 
}
int main(){
	File f[]={{1,2048,"sys.log"},
		{0,900,"test.txt"},{1,900,"access.log"}};
	int t=1;size_t z=1000;char *s="log";
	pred preds[]={{is_type,&t},
				{size_gt,&z},
				{has_name,s}};

	
	//struct ctx2 t2={is_type,size_gt,&t,&z};
	printf("{1,2048,\"sys.log\"},"
		"{0,900,\"test.txt\"},"
		"{1,900,\"access.log\"}"
		"type=1&&size>1000\n");
	for(int i=0;i<3;i++){
		//if(type_size(&f[i],&t2)){
		if(all_of(&f[i],preds,3)){
			printf("%s\n",f[i].name);
		}
	}
}

