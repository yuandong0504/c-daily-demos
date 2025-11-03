#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "model.h"
#include "list_base.h"
struct Node{
	struct Node *prev,*next;
	File data;
	bool is_sentinel;
};
void list_verify(const Node *H){
	assert(H);
	assert(H->is_sentinel);
	assert(H->prev&&H->next);
	assert(H->prev->next==H);
	assert(H->next->prev==H);

	size_t cnt_cw=0;
	Node *n=H->next;
	while(n!=H){
		cnt_cw++;
		assert(!n->is_sentinel);
		assert(n->next != n && n->prev != n);
		assert(n->next->prev==n);
		assert(n->prev->next==n);
		n=n->next;
	}
	size_t cnt_ccw=0;
	n=H->prev;
	while(n!=H){
		cnt_ccw++;
		assert(!n->is_sentinel);
		assert(n->next != n && n->prev != n);
		assert(n->prev->next==n);
		assert(n->next->prev==n);
		n=n->prev;
	}
	assert(cnt_cw==cnt_ccw);
	if(cnt_cw==0){
		assert(H->next==H&&H->prev==H);
	}else if(cnt_cw==1){
		Node *x=H->next;
		assert(x->next==H&&x->prev==H);
	}
}
void list_init(Node *H){
	H->prev=H;
	H->next=H;
	H->is_sentinel=true;
	list_verify(H);
}
bool insert_after(Node *pos,Node *x){
	assert(pos->next->prev == pos && pos->prev->next == pos);
	assert(pos && x);
	assert(!x->is_sentinel);
	assert(x->next == NULL && x->prev == NULL);
	x->next=pos->next;
	pos->next->prev=x;
	pos->next=x;
	x->prev=pos;
  	return true;
}
bool insert_before(Node *pos,Node *x){
	assert(pos->next->prev == pos && pos->prev->next == pos);
	Node *prev=pos->prev;
	return insert_after(prev,x);
}
bool list_erase(Node *x){
	assert(x&&!x->is_sentinel);
   	assert(x->next && x->prev);
	assert(x->next->prev == x && x->prev->next == x);
	x->next->prev=x->prev;
	x->prev->next=x->next;
	x->next=x->prev=NULL;
	return true;
}
static inline void file_print(const File *f){
	printf("[%d|%zu|%s]",f->type,f->size,f->name);
}
void list_print(const Node *H,const char *tag){
	printf("%s:",tag);
	const Node *n=H->next;
	if(n==H){printf("[]\n");return;}
	while(n!=H){
 		file_print(&n->data);
		n=n->next;
		if(n!=H)printf("<->");
	}
	printf("\n");
}
bool list_empty(const Node *H){return H->next==H;}
static inline Node *node_new(File data){
	Node *p=calloc(1,sizeof(*p));
	assert(p&&"OOM");
	p->data=data;
	p->is_sentinel=false;
	return p;
}
int main(void){
	Node H;
	list_init(&H);
	Node *n=node_new((File){1,900,"var/sys.log"});
	insert_after(&H,n);
	list_verify(&H);
	list_print(&H,"Insert file"
		"{{1,900,\"var/sys.log\"}} after H");

Node *n0=node_new((File){1,1024,"var/tmp.log"});
insert_before(n,n0);
list_verify(&H);
list_print(&H,"Insert 0 before 1");

Node *n2=node_new((File){0,900,"test.txt"});
insert_after(n,n2);
list_verify(&H);
list_print(&H,"Insert 2 after 1");

list_erase(n);
list_verify(&H);
list_print(&H,"erase  1");
free(n);
n=NULL;

list_erase(n2);
list_verify(&H);
list_print(&H,"erase  2");
free(n2);
n2=NULL;

list_erase(n0);
list_verify(&H);
list_print(&H,"erase  0");
free(n0);
n0=NULL;

while(!list_empty(&H)){
	Node *y=H.prev;
	list_erase(y);
	free(y);
	list_verify(&H);
}
assert(list_empty(&H));
list_print(&H,"clean up");
	return 0;
}
