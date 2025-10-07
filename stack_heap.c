#include <stdio.h>
#include <stdlib.h>
int main(void){
	int x;
	int y;
	int *z=malloc(sizeof(int));
	printf("&x=%p\n",&x);
	printf("&y=%p\n",&y);
	printf("&z=%p\n",&z);
	printf("z =%p\n",z);
	free(z);
}
