#include <stdio.h>
#include <string.h>
typedef struct{
	char name[20];
	int age;
	float gpa;
}Student;
void print_student(const Student *s){
	printf("Name:%-10s | Age:%2d | "
		"GPA:%.2f\n",s->name,s->age,
			s->gpa);
}
Student* find_top(Student *arr[],int n){
	if(n==0)return NULL;
	Student *best=arr[0];
	for(int i=0;i<n;i++){
		if(arr[i]->gpa>best->gpa){
			best=arr[i];
		}
	}
	return best;
} 
void rename_student(Student *s,const char *new_name){
	strcpy(s->name,new_name);}
int main(void){
	Student s1={"Tony",53,3.8};
	Student s2={"Bean",18,4.2};
	Student s3={"Alex",18,3.5};
	Student *arr[]={&s1,&s2,&s3};
	int n=sizeof(arr)/sizeof(arr[0]);
	printf("=== All Students ===\n");
	for(int i=0;i<n;i++){
		print_student(arr[i]);
	}
	printf("\n=== Top Student ===\n");
	Student *top=find_top(arr,n);
	if(top)print_student(top);
	printf("\n=== Rename Tony to Tony Yuan ===\n");
	rename_student(&s1,"Tony Yuan");
	print_student(&s1);
}
