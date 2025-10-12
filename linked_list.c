#include <stdio.h>
#include <stdlib.h>
struct Node{
	int val;
	struct Node* next;
 };
int main(void){
	int x;
	int ret;
	struct Node *head=NULL;
	struct Node *tail=NULL;
	printf("Enter a number to create Node,"
		"enter ctr+d to stop\n");
	while((ret=scanf("%d",&x))!= EOF){
		if(ret==1){
			printf("x=%d\n",x);
			struct Node *p=
			malloc(sizeof(struct Node));
			p->val=x;
			p->next=NULL;
			if(head==NULL){
				head=tail=p;
			}else{tail->next=p;tail=p;}
		}else scanf("%*s");
	}
	for(struct Node *n=head;n;n=n->next){
		printf("[%d]->",n->val);
	}
	printf("NULL\n");
	while(head){
		struct Node *n=head->next;
		free(head);
		head=n;
	}










/**	int x; 
	int ret;
	while((ret=scanf("%d",&x))!= EOF ){
		if(ret==0)scanf("%*s");
		else printf("x=%d,ret=%d\n",x,ret);
	}
	struct Node *a=malloc(sizeof(struct Node));
	struct Node *b=malloc(sizeof(struct Node));
	struct Node *c=malloc(sizeof(struct Node));
	a->val=10;b->val=20;c->val=30;
	a->next=b;b->next=c;
	for(struct Node* n=a;n;n=n->next){
		printf("%d\n",n->val);
	}
	free(a);free(b);free(c);
	struct Node na={1,NULL};
	struct Node nb={2,NULL};
	struct Node nc={3,NULL};
	na.next=&nb;
	nb.next=&nc;
	struct Node* p=&na;
	while(p){
		printf("%d \n",p->val);
		p=p->next;
	}
	printf("&na=%p  &nb=%p  &nc=%p\n", &na, &nb, &nc);
	printf("na.next=%p  nb.next=%p  nc.next=%p\n",
		na.next,nb.next,nc.next);
**/
}
