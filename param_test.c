#include <stdio.h>
void pass_ptr(int *ptr){
	int new_x=10;
	*ptr=new_x;
	printf("[pass_ptr]:param address=%p\n",&ptr);
}
int main(){
	int x=5;
	int *p=&x;
	printf("[main]:    param address=%p\n",&p);
	pass_ptr(p);
	printf("before x=5,now x=%d\n",x);
} 
