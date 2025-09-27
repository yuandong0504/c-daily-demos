#include <stdio.h>
void squeeze_str(char *s,char c){
	char *r=s;
	char *w=s;
	while(*r){
		if(*r!=c)*w++=*r;
		*r++;
	}
	*w='\0';
 }
int main(){
	char str[]="Con_gra_tu_la_tions!";
	printf("before squeeze:	str=%s\n",str);
	squeeze_str(str,'_');
	printf("after squeeze:	str=%s\n",str);
}
