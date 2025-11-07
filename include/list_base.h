#pragma once
#include <stdbool.h>
#include "model.h"
#include "pred_filter.h"
typedef struct Node Node;
struct Node{Node *prev,*next;File data;bool is_sentinel;};
void list_verify(const Node *H);
void list_init(Node *H);
bool insert_after(Node *pos,Node *x);
bool insert_before(Node *pos,Node *x);
bool list_erase(Node *x);
void list_print(const Node *H,const char *tag);
bool list_empty(const Node *H);
Node* node_new(File data);
void list_filter_print(const Node *H,pred *pds,
			int n,const char *tag);
size_t list_count_if(const Node *H,pred *pds,
			int n);
void list_free_all(Node *H,void(*free_data)(void *));
