#pragma once
#include <stdbool.h>
#include "model.h"
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
