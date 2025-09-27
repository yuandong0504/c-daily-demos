#include <stdio.h>

void print_matrix(char *title,int row,int cols,int (*arr)[cols]){
	printf("%s\n",title);
	for(int i=0;i<row;i++){
		for(int j=0;j<cols;j++){
			printf("%3d",*(*(arr+i)+j));
		}
		printf("\n");
	}
}
void print_m_linear(char *title,int r,int c,int(*m)[c]){
	int *p=&m[0][0]; 
	int total=r*c;
	int n=0;
	printf("%s\n",title);
	while(n<total){
		printf("%3d",*p++);
		if(++n%c==0)printf("\n");
	}
}

void main(){
	int a[2][3]={{1,2,3},{4,5,6}};
	int r=2;
	int c=3;
 	//print_matrix("A",r,c,a);
	print_m_linear("A_L",r,c,a);
}
