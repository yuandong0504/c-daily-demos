#include <stdio.h>
void print_bubble(int arr[]){
	printf("%d,%d,%d,%d,%d\n",arr[0],arr[1],arr[2],arr[3],arr[4]);
}
int main(){
	int a[5]={5,9,6,4,8};
	int n=5;
	printf("init:	");
	print_bubble(a);
	for(int i=0;i<n-1;i++){
		for(int j=0;j<n-1-i;j++){
			if(a[j]>a[j+1]){
				int tmp=a[j];
				a[j]=a[j+1];
				a[j+1]=tmp;
				printf("i:%d	",i);
				print_bubble(a);
			}
		}
	}
 }
