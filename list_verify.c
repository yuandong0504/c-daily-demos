#include <stdio.h>
#include <assert.h>
struct Node{
	int val;
	struct Node *prev;
	struct Node *next;
};
struct List{
	struct Node *head;
	struct Node *tail;
};
void verify(const struct List * list){
	struct Node *head=list->head;
	struct Node *tail=list->tail;
	assert((head==NULL)==(tail==NULL));
	if(!head)return;
	assert(head->prev==NULL);
	assert(tail->next==NULL);
	struct Node *last=NULL;
	for(struct Node *n=head;n;n=n->next){
		if(n->prev)assert(n->prev->next==n);
		if(n->next)assert(n->next->prev==n);
		last=n;
	}
	assert(last==tail);
}
int delete(struct Node **h,struct Node **t,int val){
	struct Node *head=*h;
	struct Node *tail=*t;
	for(struct Node *n=head;n;n=n->next){
		if(n->val==val){
			if(n->prev)
				n->prev->next=n->next;
			else *h=n->next;
			if(n->next)
				n->next->prev=n->prev;
			else *t=n->prev;
			n=NULL;
			return 1;
		}
	}
	return 0;
}
void print(struct Node *head){
	if(!head){printf("NULL\n");return;}
	for(struct Node *n=head;n;n=n->next){
		printf("[%d]",n->val);
		if(n->next)printf("<->");
	}
	printf("\n");
}
void reverse(struct Node **h,struct Node **t){
	struct Node *n=*h;
	while(n){
		struct Node *tmp=n->next;
		if(!(n->prev))*t=n;
		if(!(n->next))*h=n;
		n->next=n->prev;
		n->prev=tmp;
		n=tmp;
	}
}
void append(struct Node **h,struct Node **t,
				struct Node **n){
	struct Node *tail=*t;
	if(tail){
		tail->next=*n;
 		(*n)->prev=tail;
	}
	*t=*n;
	if(!(*h))*h=*n;
}
int main(void){
	struct Node a={1,NULL,NULL};
	struct Node b={2,NULL,NULL};
	struct Node c={3,NULL,NULL};
	a.next=&b;b.next=&c;
	c.prev=&b;b.prev=&a;

	struct Node *head=&a;
	struct Node *tail=&c;
	struct List list={head,tail};

	printf("init:\n");
	print(head);
	reverse(&head,&tail);
	list.head=head;
	list.tail=tail;
	verify(&list);
	printf("after reverse:\n");
	print(head);

	printf("before delete:\n");
	print(head);
	int x=3;
	delete(&head,&tail,x);
	list.head=head;
	list.tail=tail;
	verify(&list);
	printf("after delete %d:\n",x);
	print(head);

	x=2;
	delete(&head,&tail,x);
	list.head=head;
	list.tail=tail;
	verify(&list);
	printf("after delete %d:\n",x);
	print(head);

	x=1;
	delete(&head,&tail,x);
	list.head=head;
	list.tail=tail;
	verify(&list);
	printf("after delete %d:\n",x);
	print(head);

	struct Node d={4,NULL,NULL};
	struct Node *nd=&d;
	append(&head,&tail,&nd);
	list.head=head;
	list.tail=tail;
	verify(&list);
	printf("after append 4:\n");
	print(head);

	struct Node e={5,NULL,NULL};
	nd=&e;
	append(&head,&tail,&nd);
	list.head=head;
	list.tail=tail;
	verify(&list);
	printf("after append 5:\n");
	print(head);

	return 0;
 
}
