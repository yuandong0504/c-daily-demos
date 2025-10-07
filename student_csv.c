#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CSV_PATH "students.csv"
#define TMP_PATH "students.csv.tmp"
typedef struct{char city[20];int zip;}Address;
typedef struct{
	char name[20];
	int age;
	Address addr;
}Student;
typedef struct Node{
	Student data;
	struct Node *next;
}Node;
Node* create_node(Student s){
	Node *p=malloc(sizeof(*p));
	if(!p){perror("malloc");exit(1);}
	p->data=s;
	p->next=NULL;
	return p;
}
void append_tail(Node** head, Student s){
	Node *n=create_node(s);
	if(!*head){*head=n;return;}
	Node *c=*head;
	while(c->next){c=c->next;}
	c->next=n;
}
int load_csv(Node **head,const char *path){
	FILE *fp=fopen(path,"r");
	if(!fp){perror("fopen:");exit(1);}
	char line[256];
	fgets(line,sizeof(line),fp);
	while(fgets(line,sizeof(line),fp)){
		Student s={0};
		if(sscanf(line,"%19[^,],%d,"
			"%19[^,],%d",s.name,&s.age,
			s.addr.city,&s.addr.zip)
			==4){
			append_tail(head,s);
		}
	}
	fclose(fp);
	return 0;
}
void print_csv_head(){
	printf("\n===       Students Info        ===\n");
	printf("   name   |age|     city     | zip\n");
}
void print_all(Node *head){
	print_csv_head();
	for(Node *c=head;c;c=c->next){
		printf("%-10s|%3d|%-14s|%d\n",
			c->data.name,
			c->data.age,
			c->data.addr.city,
			c->data.addr.zip);
	}
}
void print_one(Node *head){
	print_csv_head();
	if(head){
		printf("%-10s|%3d|%-14s|%d\n",
			head->data.name,
			head->data.age,
			head->data.addr.city,
			head->data.addr.zip);
	}
}
void save_csv(Node *head,const char *path){
	FILE *fp=fopen(TMP_PATH,"w");
	if(!fp){perror("fopen:");exit(1);}
	fprintf(fp,"name,age,city,zip\n");
	for(Node *n=head;n;n=n->next){
		fprintf(fp,"%s,%d,%s,%d\n",
			n->data.name,
			n->data.age,
			n->data.addr.city,
			n->data.addr.zip);
	}
	fclose(fp);
	rename(TMP_PATH,path);
}
void free_all(Node* head){
	while(head){
		Node *n=head->next;
		free(head);
		head=n;
	}
}
Node* find_by_name(Node* head,char* name){
	for(Node *n=head;n;n=n->next){
		if(strcmp(n->data.name,name)==0){
			return n;}}
	return NULL;
}
int main(void){
	Node *head=NULL;
	load_csv(&head,CSV_PATH);
	print_all(head);
	Student s={"Rudy",52,{"Corona",92882}};
	if(!find_by_name(head,"Rudy")){
		append_tail(&head,s);
	}
	save_csv(head,CSV_PATH);
	print_all(head);
	print_one(find_by_name(head,"Rudy"));
	free_all(head);
}
