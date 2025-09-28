#include <stdio.h>
struct Address {char city[20];
		int zip;};
struct Student {char name[20];
		int age;
		struct Address addr;};
int main(){
	struct Student s={"Tony Yuan",53,
			{"La Puente",91744}};
	printf("%s:%d:%s:%d\n",s.name,s.age,
		s.addr.city,s.addr.zip);
}
