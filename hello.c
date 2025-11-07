#include <stdio.h>
void print_bits(int v){
	for(int i=7;i>=0;i--){
		printf("%d",(v>>i)&1);
	}
}
int main(){
	int action=69;
	printf("69:0100 0101\n");
	print_bits(69);puts(" ");
}
