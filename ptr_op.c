#include <stdio.h>
#include <stdlib.h>
void create_hp(int **ptr){
	*ptr=malloc(sizeof(int));
	**ptr=20;
}
int main(void){
	int *p=NULL;
	create_hp(&p);
	printf("*p=%d\n",*p);
	free(p);
}
