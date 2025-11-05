#include <stdio.h>
#include <stdlib.h>
#include "list_base.h"
int main(void){
	Node H;
	list_init(&H);
	Node *n0=node_new((File){1,1024,"sys.log"});
	insert_after(&H,n0);
	Node *n1=node_new((File){0,512,"test.txt"});
	insert_after(n0,n1);
	Node *n2=node_new((File){1,800,"test.log"});
	insert_after(n1,n2);
	list_print(&H,"all nodes:\n");
	int t=1;size_t z=1000;char *name="log";
	pred fds[]={{is_type,&t},{size_gt,&z},
			{has_name,name}};
	list_filter_print(&H,fds,3,"type=1,size>1000,"
			"has name\"log\"");

	printf("total count of filtered file:%zu\n",
		list_count_if(&H,fds,3));
	
	list_erase(n0);
	free(n0);
	list_erase(n1);
	free(n1);
	list_erase(n2);
	free(n2);
	return 0;
}
