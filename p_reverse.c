#include <stdio.h>
void swap(char *a,char *b){
	char tmp=*a;
	*a=*b;
	*b=tmp;
}
void reverse(char *str){
	char *p=str;
	char *end=str;
	while(*end)end++;
	if(end>p)end--;
	while(p<end){
		swap(p++,end--); 
	}
}
void main(){

	char s[]="hello world!";
	printf("before reverse:	%s\n",s);
	reverse(s);
	printf("after reverse:	%s\n",s);
}
