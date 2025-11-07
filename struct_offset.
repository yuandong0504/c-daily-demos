#include <stdio.h>
#include <stddef.h>

struct S{
	char c;
	int i;
	long d;
};
int main(void){
	struct S s={0};
	s.c='a';
	s.i=4;
	s.d=8;
	printf("(size_t)-1=%zu\n",(size_t)-1);
	unsigned char *ps=(unsigned char*)&s;
	for(int i=0;i<sizeof(struct S);i++){
		printf("%02X ",ps[i]);
	}


	printf("\nIn struct S,offset of i=%zu\n",
		(size_t)&(((struct S*)0)->i));
	unsigned char *p=(unsigned char*)&s;
	unsigned char *end=p+sizeof(struct S);
	for(;p<end;p++){
		printf("%02X ",*p);
	}
	printf("\n\n");
	printf("offsetof(struct S,c)=%zu\n",
		 offsetof(struct S,c));
	printf("offsetof(struct S,i)=%zu\n",
		 offsetof(struct S,i));
	printf("offsetof(struct S,d)=%zu\n",
		offsetof(struct S,d));
	printf("%p %p %p\n",&s.c,&s.i,&s.d);
}
