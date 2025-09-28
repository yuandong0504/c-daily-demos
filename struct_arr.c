#include <stdio.h>
struct Student{
	char name[20];
	int age;
};

void print(struct Student s){
	printf("%s:%d\n",s.name,s.age);
}
int main(){
	struct Student student[2]={
		{"Tony Yuan",53},
		{"Bean Yuan",18}};
	for(int i=0;i<2;i++){
		print(student[i]);
	}
}
