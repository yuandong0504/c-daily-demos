#include <stdio.h>

int main(void){
	char name[20];
	int age;
	float gpa;
	printf("Please enter name age gpa:\n");
	scanf("%s %d %f",name,&age,&gpa);
	printf("Name:%s age:%d gpa:%.2f\n",name,age,gpa);
}
