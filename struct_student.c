#include <stdio.h>
struct Student{
	char name[20];
	int age;
};
void print(struct Student s){
	printf("%s:%d\n",s.name,s.age);
}
int main(){
	struct Student student=
		{"Bean Yuan",18};
	print(student);
}
