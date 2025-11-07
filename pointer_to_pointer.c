#include <stdio.h>
struct S {int val;struct S *next;};
int main(void){
	struct S a={1,NULL},b={2,NULL},c={3,NULL};
	a.next=&b;
	b.next=&c;
	printf("address of a		=%p\n",&a);
	printf("address of a.val	=%p\n",&(a.val));
	printf("address of a.next	=%p\n",&(a.next));
	printf("a.next			=%p\n",a.next);

	printf("\naddress of b		=%p\n",&b);
	printf("address of c		=%p\n",&c);
	struct S *p=&a,**pp=&p;
	printf("\npp			=%p\n",pp);
	pp=&(*pp)->next;
	printf("\nafter pp=&(*pp)->next:\n");
	printf("pp			=%p\n",pp);
	printf("a.next			=%p\n",a.next);
	*pp=(*pp)->next;
	printf("\nafter *pp=(*pp)->next:\n");
	printf("a.next			=%p\n",a.next);
}
