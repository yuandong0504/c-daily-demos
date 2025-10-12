#include <stdio.h>
#include <stddef.h>
#define container_of(ptr,type,member)\
	 (type*)((char*)(ptr)-offsetof(type,member))
struct S {
	int i;
	char c;
	double d;
};
int main(void){
	struct S s={0};
	double *pd=&s.d;
	printf("the address of pd	=%p\n",pd);
	printf("&s from container_of	=%p\n",
		container_of(pd,struct S,d));
	printf("the address of &s	=%p\n",&s);
}
