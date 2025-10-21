#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
typedef struct Node{
	struct Node *prev,*next;
	int val;
	int is_sentinel;
}Node;
void list_verify(Node *H){
	assert(H);
	assert(H->is_sentinel==1);
	assert(H->prev&&H->next);
	assert(H->prev->next==H);
	assert(H->next->prev==H);

	size_t cnt_cw=0;
	Node *n=H->next;
	while(n!=H){
		cnt_cw++;
		assert(n->is_sentinel!=1);
		assert(n->next->prev==n);
		assert(n->prev->next==n);
		n=n->next;
	}
	size_t cnt_ccw=0;
	n=H->prev;
	while(n!=H){
		cnt_ccw++;
		assert(n->is_sentinel!=1);
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
	H->is_sentinel=1;
	list_verify(H);
}
int insert_after(Node *pos,Node *x){
	x->next=pos->next;
	pos->next->prev=x;
	pos->next=x;
	x->prev=pos;
  	return 1;
}
int insert_before(Node *pos,Node *x){
	Node *prev=pos->prev;
	insert_after(prev,x);
	return 1;
}
int list_erase(Node *x){
	assert(x&&x->is_sentinel!=1);
	x->next->prev=x->prev;
	x->prev->next=x->next;
	x->next=x->prev=NULL;
	return 1;
}
void list_print(Node *H,const char *tag){
	printf("%s:",tag);
	Node *n=H->next;
	if(n==H){printf("[]\n");return;}
	while(n!=H){
		printf("[%d]",n->val);
		n=n->next;
		if(n!=H)printf("<->");
	}
	printf("\n");
}
int list_empty(Node *H){return H->next==H;}
static inline Node *node_new(int val){
	Node *p=calloc(1,sizeof(*p));
	assert(p&&"OOM");
	p->val=val;
	return p;
}
int main(void){
	Node H;
	list_init(&H);
	Node *n=node_new(1);
	insert_after(&H,n);
	list_verify(&H);
	list_print(&H,"Insert 1 after H");

Node *n0=node_new(0);
insert_before(n,n0);
list_verify(&H);
list_print(&H,"Insert 0 before 1");

Node *n2=node_new(2);
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

for(int i=0;i<5;++i){
	Node *x=node_new(i*10);
	insert_before(&H,x);
	list_verify(&H);
}
list_print(&H,"Insert 5 nodes");

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
