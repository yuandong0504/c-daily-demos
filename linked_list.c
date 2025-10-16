#include <stdio.h>
#include <stdlib.h>
struct Node{
	int val;
	struct Node* next;
};
struct Node* create(int val){
	struct Node *p=malloc(sizeof(*p));
	if(!p){perror("[malloc]:");exit(1);}
	p->val=val;
	p->next=NULL;
	return p;
}
void print_all(struct Node *head){
	for(struct Node *n=head;n;n=n->next){
		 printf("[%d]->",n->val);
	}
	printf("NULL\n");
}
void reverse(struct Node **head){
	struct Node *prev=NULL,*cur=*head,*next=NULL;
	while(cur){
		next=cur->next;
		cur->next=prev;
		prev=cur;
		cur=next;
	}
	*head=prev;
}
/**void print_reverse(struct Node *head){
	if(!head)return;
	print_reverse(head->next);
	printf("[%d]->",head->val);
}**/
void print_reverse(struct Node *head,int depth){
	if(!head)return;
	printf("%*s🟩  [%d]\n",depth*2," ",head->val);
	print_reverse(head->next,depth+1);
	printf("%*s⬜  [%d]\n",depth*2," ",head->val);
}
struct Node* find_first(struct Node *head,int val){
	struct Node *n=head;
	for(;n;n=n->next){
		if(n->val==val){return n;}
	}
	return NULL;
}
int insert_after(struct Node **head,int front,int after){
	struct Node *p=find_first(*head,front);
	if(!p)return 0;
	struct Node *n=create(after);
	struct Node *next=p->next;
	p->next=n;
	n->next=next;
	return 1;
}
/**int delete_first(struct Node **head,int val){
	struct Node *prev=NULL,*cur=*head;
	while(cur){
		if(cur->val==val){
			if(prev==NULL)*head=cur->next;
			else prev->next=cur->next;
			free(cur);
			return 1;
		}
		prev=cur;cur=cur->next;
	}
	return 0;
}**/
int delete_first(struct Node **head,int val){
	struct Node **pp=head;
	while(*pp&&(*pp)->val!=val)
		pp=&(*pp)->next;
	if(*pp){
		struct Node *victim=*pp;
		*pp=victim->next;
		free(victim);		
		return 1;
	}
	return 0;
}
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
			struct Node *p=create(x);
			if(head==NULL){
				head=tail=p;
			}else{tail->next=p;tail=p;}
		}else scanf("%*s");
	}
	print_all(head);
	/**reverse(&head);
	print_all(head);**/
	printf("\n=== print reverse by stack === \n");
	//print_reverse(head);
	print_reverse(head,0);
//	printf("[NULL]\n");
	printf("=== end of print ===\n\n");

	/**int f;
	int a;
	printf("enter 2 number:");
	clearerr(stdin);
	scanf("%d %d",&f,&a);
	if(insert_after(&head,f,a)==1){
		printf("insert %d after %d\n",a,f);
		print_all(head);}
	else printf("not found or failed.\n");
	printf("enter a number to delete:\n");
	int d;
	while(scanf("%d",&d)==1){
		if(delete_first(&head,d)==1)
			print_all(head);
		else printf("node doesn't exist.\n");
	}**/
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
