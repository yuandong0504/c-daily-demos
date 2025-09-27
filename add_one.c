#include <stdio.h>
void add_one(int *p){
	(*p)++;
}
int main(){
	int x=10;
	int *ptr;
	ptr=&x;
	printf("before 	add,x=%d,ptr=%p\n",x,ptr);
	add_one(ptr);
	printf("after	add,x=%d,ptr=%p\n",x,ptr);
	return 0;
}
