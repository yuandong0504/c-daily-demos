#include <stdio.h>
void print_arr2d(int (*a)[3],int rows){
	for(int i=0;i<rows;i++){
		for(int j=0;j<3;j++){
			printf("&a[%d][%d]=%p value=%d\n",i,j,&a[i][j],a[i][j]);
		}
	}
}
int main(){
	int arr[2][3]={{1,2,3},{4,5,6}};
	print_arr2d(arr,2);
}

