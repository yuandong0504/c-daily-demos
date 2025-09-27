#include <stdio.h>
void swap(int *px,int *py){
	int tmp;
	tmp=*px;
	*px=*py;
	*py=tmp;
}
int main(){
	int x=10;
	int y=20;
	printf("before swap:	x=%d,y=%d,&x=%p,&y=%p\n",x,y,&x,&y);
	swap(&x,&y);
	printf("after swap:	x=%d,y=%d,&x=%p,&y=%p\n",x,y,&x,&y);
	return 0;
}
