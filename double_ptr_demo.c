#include <stdio.h>
int main(){
	char *str[]={"apple","berry","banana","grape",NULL};
	for(char **p=str;*p!=NULL;p++){
		printf("%s\n",*p);
	}
}
