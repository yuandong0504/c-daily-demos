#include <stdio.h>
//#pragma pack(push,1)
struct S{
	int x;
	char c;
	float f;
	double d;
};
//#pragma pack(pop)
int main(void){
	int x=5;
	char c='h';
	float f=3.14;
	double d=2;
	struct S s={10,'a',5.25,4};
	int *px=&x;
	char *pc=&c;
	float *pf=&f;
	struct S *ps=&s;
	double *pd=&d;
	printf("sizeof(x)=%zu\n",sizeof(x));
	printf("sizeof(c)=%zu\n",sizeof(c));
	printf("sizeof(f)=%zu\n",sizeof(f));
	printf("sizeof(d)=%zu\n",sizeof(d));
	printf("sizeof(s)=%zu\n",sizeof(s));
	printf("ptr x diff=%td\n",
		(char*)(px+1)-(char*)px);
	printf("ptr c diff=%td\n",
		(char*)(pc+1)-(char*)pc);
	printf("ptr f diff=%td\n",
		(char*)(pf+1)-(char*)pf);
	printf("ptr d diff=%td\n",
		(char*)(pd+1)-(char*)pd);
	printf("ptr s diff=%td\n",
		(char*)(ps+1)-(char*)ps);


	printf("sizeof(void*)*8=%zu\n",
		sizeof(void*)*8);
	printf("c=%c\n",c);
	printf("c=0x%02X\n",(unsigned char)c);
	printf("&c=%p\n",(void*)&c);
	printf("pc=%p\n",(void*)pc);

	int a[3]={10,20,30};
	printf("(a+2)-a=%td\n",(a+2)-a);
	printf("(char*)(a+2)-(char*)a=%td\n",
		(char*)(a+2)-(char*)a);


}


