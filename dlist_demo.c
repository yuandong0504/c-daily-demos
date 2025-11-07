
#include <stdio.h>
struct Node {
	int val;
	struct Node *prev;
	struct Node *next;
}; 
void print_head(struct Node *head){
	printf("\nprint from head:\n");
	for(struct Node *n=head;n;n=n->next){
		printf("[%d]<->",n->val);
	}
	printf("\n\n");
}
void print_tail(struct Node *tail){
	printf("print from tail:\n");
	for(struct Node *n=tail;n;n=n->prev){
		printf("[%d]<->",n->val);
	}
 	printf("\n\n");
}
int insert_after(struct Node *pos,struct Node *n){
	if(pos->next){
		pos->next->prev=n;
		n->next=pos->next;
	}
	pos->next=n;
	n->prev=pos;
	return 1;
}
int delete_one(struct Node **h,struct Node **t,
			struct Node **n){
	if((*n)->prev){
		(*n)->prev->next=(*n)->next;
	}else {*h=(*n)->next;(*h)->prev=NULL;}
	if((*n)->next){
		(*n)->next->prev=(*n)->prev;
	}else {*t=(*n)->prev;(*t)->next=NULL;}
	return 1;
}
int reverse(struct Node **head,struct Node **tail){
	struct Node *tmp=NULL;
	struct Node *n=*head;
	*tail=*head;
	while(n){
		tmp=n->next;
		n->next=n->prev;
		n->prev=tmp;
		if(!tmp){
			*head=n;
			break;
		}
		n=tmp;
	}
	return 1;
}
int main(void){
	struct Node a={1,NULL,NULL};
	struct Node b={2,NULL,NULL};
	struct Node c={3,NULL,NULL};
	a.next=&b;
	b.prev=&a;
	b.next=&c;
	c.prev=&b;

	print_head(&a);
	print_tail(&c);

	struct Node d={0,NULL,NULL};
	insert_after(&a,&d);
	printf("[0] insert after [1]:\n");
	print_head(&a);
	print_tail(&c);

	struct Node *head=&a,*tail=&c;
	printf("original doubly linked list:\n");
	print_head(head);
	reverse(&head,&tail);
	printf("after reverse::\n");
	print_head(head);
	print_tail(tail);
	
/**	struct Node *head=&a,*tail=&c,*p=&d;
	delete_one(&head,&tail,&p);
	printf("after delete [0]:\n");
	print_head(head);
	print_tail(tail);
p=&a;
delete_one(&head,&tail,&p);
printf("after delete head:\n");
print_head(head);
print_tail(tail);

p=&c;
delete_one(&head,&tail,&p);
printf("after delete tail:\n");
print_head(head);
print_tail(tail);
**/
}

