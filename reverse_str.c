#include <stdio.h>
void reverse_str(char *s){
	char *L=s;
	char *R=s;
	while(*R) R++;
	R--;
	while(L<R){
		char tmp=*L;
		*L=*R;
		*R=tmp;
		L++;
		R--;
	}
}

int main(){
	char str[]="Congratulations";
	printf("before:	%s\n",str);
	reverse_str(str);
	printf("after:	%s\n",str);
}
