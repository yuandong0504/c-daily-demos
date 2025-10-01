#include <stdio.h>
#include <stdlib.h>
typedef struct {
	char name[20];
	int age;
	float gpa;
} Student;

int main(void){
	int n;
	printf("=== Input the numbers "
			"of students ===\n");
	scanf("%d",&n);
	Student *arr=malloc(n*sizeof(Student));
	printf("\nEnter name age gpa:\n");
	for(int i=0;i<n;i++){
		scanf("%s %d %f",arr[i].name,
			&arr[i].age,&arr[i].gpa);
	}
	for(int i=0;i<n;i++){
		printf("%-10s | %d | %.2f \n",arr[i]. name,
			arr[i].age,arr[i].gpa);
	}
	free(arr);
}
