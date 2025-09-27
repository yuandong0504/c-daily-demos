#include <stdio.h>
void by_value(int x) {x=99;}
void by_pointer(int *p) {*p=99;}
void main(){
	int a=5;
	int *pt=&a;
	by_value(a);
	printf("after by_value:		a=%d\n",a);
	by_pointer(pt);
	printf("after by_pointer:	a=%d\n",a);
}
