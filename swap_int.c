#include <stdio.h>
void swap(int *a, int *b){
	int tmp=*a;
	*a=*b;
	*b=tmp;
}

void main(){
	int x=5;
	int y=10;
	printf("before swap:	x=%d,y=%d\n",x,y);
	swap(&x,&y);
	printf("after swap:	x=%d,y=%d\n",x,y);
}
