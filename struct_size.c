#include <stdio.h>
struct A {char c; int x;};
struct B {int x; char c;};
int main(void){
	printf("sizeof struct A:%zu\n"
		"sizeof struct B:%zu\n",
		sizeof(struct A),sizeof(struct B));
}

