#include "list_base.h"
#include <stdio.h>
#include <assert.h>
int main(void){
	Node H;
	list_init(&H);
	Node *n0=node_new((File){1,512,"sys.log"});
	Node *n1=node_new((File){0,1024,"sys.txt"});
	Node *n2=node_new((File){1,900,"log.log"});
	insert_after(&H,n0);
	insert_after(n0,n1);
	insert_after(n1,n2);	
	list_print(&H,"3 Nodes");
	list_free_all(&H,NULL);
	list_verify(&H);
	assert(list_empty(&H));
	list_print(&H,"empty");
	return 0;
}
