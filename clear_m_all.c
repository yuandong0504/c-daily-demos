#include <stdio.h>
void print_m(char *title,int row,int col,int(*m)[col]){
	printf("%s\n",title);
	int *p=&m[0][0];
	int *end=p+row*col;
	int c=0;
	while(p<end){
		printf("%3d",*p++);
		if(++c==col){
			printf("\n");
			c=0;
		}
	}
}
void clear_all(int row,int col,int (*m)[col]){
	int *p=&m[0][0];
	int *end=p+row*col;
	while(p<end){
		*p++=0;
	}
}
void main(){
	int a[2][3]={{1,2,3},{4,5,6}};
	int r=2;
	int c=3;
	print_m("before:",r,c,a);
	clear_all(r,c,a);
	print_m("after_clear:",r,c,a);
}
