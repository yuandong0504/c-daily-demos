#include <stdio.h>
#include <stdlib.h>
typedef struct Node{
	int val;
	struct Node *next;
}Node;
void print_list(Node *head){
	Node *cur=head;
	while(cur){
		printf("%2d-> ",cur->val);
		cur=cur->next;
	}
	printf("NULL\n");
}
void append(Node *head,Node *new_node){
	Node *cur=head;
	while(cur->next){cur=cur->next;}
	cur->next=new_node;
}
Node* create_node(int val){
	Node *p=malloc(sizeof(Node));
	p->val=val;
	p->next=NULL;
	return p;
}
void free_list(Node *head){
	Node *cur=head;
	while(cur){
		Node *next=cur->next;
		free(cur);
		cur=next;
	}
}
void delete_node(Node **head,int val){
	Node *cur=*head,*prev=NULL;
	while(cur){
		if(cur->val==val){
			if(prev){
				prev->next=cur->next;
			}else{
				*head=cur->next;
			}
			printf("Delete val:%d\n",val);
			print_list(*head);
			free(cur);
			return;
		}
		prev=cur;
		cur=cur->next;
	}
}
void insert_node(Node **head,int val){
	Node *cur=*head,*prev=NULL;
	Node *new_node=create_node(val);
	while(cur){
		if(cur->val>val){
			if(prev){
				prev->next=new_node;
			}else{
				*head=new_node;
			}
			printf("Insert Node:%d\n",val);
			new_node->next=cur;
			print_list(*head);
			return;
		}
		prev=cur;
		cur=cur->next;
	}
	printf("Insert Node:%d\n",val);
	prev->next=new_node;
	print_list(*head);
}
int main(void){
	Node *a=create_node(100);
	Node *b=create_node(200);
	Node *c=create_node(300);
	append(a,b);
	append(a,c);
	Node *d=create_node(400);
	append(a,d);
	Node *e=create_node(500);
	append(a,e);
 	print_list(a);
	delete_node(&a,200);
	delete_node(&a,100);
	delete_node(&a,500);
	insert_node(&a,50);
	insert_node(&a,250);
	insert_node(&a,550);
	free_list(a);
}
