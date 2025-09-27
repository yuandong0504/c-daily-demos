#include <stdio.h>
#include <string.h>
struct Student{
	char name[20];
	int age;
};
void print(struct Student *s){
	printf("%s:%d\n",s->name,s->age);
}
void edit(struct Student *s){
	strcpy(s->name,"Tony Yuan");
	s->age=53;
}
int main(){
	struct Student student=
		{"Bean Yuan",18};
	print(&student);
	edit(&student);
	print(&student);
}
