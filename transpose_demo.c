#include <stdio.h>

#define R 3
#define C 3

void print_matrix(const char* title,int rows, int cols,int(*m)[cols]){
	printf("%s\n",title);
	for(int i=0;i<rows;i++){
		for(int j=0;j<cols;j++){
			printf("%3d",m[i][j]);
		}
		printf("\n");
	}
}

void transpose(int r, int c, int (*in)[c],int (*out)[r]){
	for(int i=0;i<r;i++){
		for(int j=0;j<c;j++){
			out[j][i]=in[i][j];
		}
	}
}
int main(){
	int a[R][C]={{1,2,3},{4,5,6},{7,8,9}};
	int b[C][R];
	print_matrix("A=",R,C,a);
	transpose(R,C,a,b);
	print_matrix("A^T=",C,R,b);
	return 0;

}
