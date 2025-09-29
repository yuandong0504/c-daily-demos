#include <stdio.h>
#include <stddef.h>
struct A {char c; int x;};
struct B {int x; char c;};
int main(void){
	printf("sizeof struct A:%zu\n"
		"sizeof struct B:%zu\n",
		sizeof(struct A),sizeof(struct B));
	printf("A.c @ %zu,A.x @ %zu\n",
		offsetof(struct A,c),
		offsetof(struct A,x));
	printf("B.c @ %zu,B.x @ %zu\n",
		offsetof(struct B,c),
		offsetof(struct B,x));
}

