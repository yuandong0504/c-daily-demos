#include <stdio.h>
#include <stdlib.h>
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
void print_all(Node *head){
	printf(" ===      Students Info      ===\n");
	printf("   name   |age|   city   | zip  \n");
	for(Node *c=head;c;c=c->next){
		printf("%-10s|%3d|%-10s|%d\n",
			c->data.name,
			c->data.age,
			c->data.addr.city,
			c->data.addr.zip);
	}
}
void save_csv(Node *head,const char *path){
	char tmp[]="students_csv.tmp";
	FILE *fp=fopen(tmp,"w");
	if(!fp){perror("fopen:");exit(1);}
	fprintf(fp,"name,age,city,zip\n");
	for(Node *n=head;n;n=n->next){
		fprintf(fp,"%s,%d,%s,%d\n",
			n->data.name,
			n->data.age,
			n->data.addr.city,
			n->data.addr.zip);
	}
	rename(tmp,path);
	fclose(fp);
}
void free_all(Node* head){
	while(head){
		Node *n=head->next;
		free(head);
		head=n;
	}
}
Student* find_by_name(Node** head,char* name){
	return NULL;
}
int main(void){
	Node *head=NULL;
	char path[]="students.csv";
	load_csv(&head,path);
	print_all(head);
	Student s={"Rudy",52,{"Corona",92882}};
	append_tail(&head,s);
	save_csv(head,path);
	print_all(head);
	free_all(head);
}
