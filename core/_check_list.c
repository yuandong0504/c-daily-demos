#include "list_base.h"
#include <stdio.h>
#include <stdlib.h>
int main(void){
	Node H;
	list_init(&H);
	list_verify(&H);
	Node  *n0=node_new((File){1,900,"sys.log"});
	insert_after(&H,n0);
	list_print(&H,"insert n0");
	list_verify(&H);
	Node  *n1=node_new((File){0,1200,"test.txt"});
	insert_before(&H,n1);
	list_print(&H,"insert n1");
	list_verify(&H);
	list_erase(n1);
	list_print(&H,"erase n1");
	list_verify(&H);
	free(n1);
	list_erase(n0);
 	list_print(&H,"erase n0");
 	list_verify(&H);
	free(n0);
	printf(list_empty(&H)?"list is empty":"!empty");
	puts("");
	return 0;
}
