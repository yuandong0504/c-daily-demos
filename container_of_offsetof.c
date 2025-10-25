#include <stdio.h>
#define offsetof(Type,member)\
		(size_t)(&((Type*)0)->member)
#define container_of(ptr,Type,member)\
		(Type*)(((char*)(ptr))-offsetof(Type,member))
typedef struct Student{
	char *name;
	int age;
}Student; 

int main(){
	printf("offsetof(age)=%zu\n",
			offsetof(Student,age));
	Student s={"Tony",53};
	int *i=&(s.age);
	Student *p=container_of(i,Student,age);
	printf("name from container_of:%s\n",
					p->name);
}
