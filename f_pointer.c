#include <stdio.h>
void say_f_pointer(){
	printf("Hello,function pointer\n");
}
void p_redirect(){
	printf("Hello,pointer redirected\n");
}
int main(){
	void (*sfp)(void)=say_f_pointer;
	sfp();

	typedef void (*a_sfp)(void);
	a_sfp fp=say_f_pointer; 
	fp(); 

	fp=p_redirect;
	fp();
}
