#include <stdio.h>
void double_all(int *arr, int n){
	for(int i=0;i<n;i++){
		*arr *=2;
		arr++;
	}
}
void print_arr(int *arr, int n){
	for(int i=0;i<n;i++){
		printf("%d ",*arr);
		arr++;
	}
	printf("\n");
}

void main(){
	int a[3]={1,2,3};
	int size=3;
	printf("before double:\n");
	print_arr(a,size);
	double_all(a,size);
	printf("after double:\n");
	print_arr(a,size);
}

