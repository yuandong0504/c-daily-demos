#include <stdio.h>
#include <stdbool.h>
typedef bool (*predicate_fn)(int v,void *ctx);
bool gt(int v,void *ctx){int i=*(int*)ctx;return v>i;}
bool even(int v,void *ctx){(void)ctx;return v%2==0;}
bool lt(int v,void *ctx){int i=*(int*)ctx;return v<i;}
int main(){
	predicate_fn p=gt;
	int v=7,t=10;
	printf("gt(7,10)=%s\n",p(v,&t)?"True":"False");
	p=even;
	printf("even(7,NULL)=%s\n",p(v,NULL)?"True":"False");
	p=lt;
	printf("lt(7,10)=%s\n",p(v,&t)?"True":"False");
} 
