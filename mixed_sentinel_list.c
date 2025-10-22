// gcc -Wall -Wextra -O2 mixed_sentinel_list.c -o mixed_list && ./mixed_list
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

/* ================= 基础双向循环链表（哨兵） ================= */
typedef struct Node { struct Node *prev, *next; } Node;

static inline void list_init(Node *H){ H->next=H; H->prev=H; }
static inline bool list_empty(Node *H){ return H->next==H; }

static inline void list_insert_after(Node *pos, Node *x){
    assert(pos && x);
    assert(x->next==NULL && x->prev==NULL);  // 未挂接的“游离状态”
    x->next = pos->next;
    x->prev = pos;
    pos->next->prev = x;
    pos->next = x;
}
static inline void list_insert_before(Node *pos, Node *x){
    list_insert_after(pos->prev, x);
}
static inline void list_erase(Node *x){
    assert(x && x->next && x->prev);
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x->prev = NULL; // 置游离，防二次删除
}

/* =============== 自证（verify） =============== */
static void list_verify(Node *H){
    assert(H && H->next && H->prev);
    assert(H->next->prev==H && H->prev->next==H);
    size_t cw=0, ccw=0;

    for(Node *n=H->next; n!=H; n=n->next){
        cw++;
        assert(n->next && n->prev);
        assert(n->next->prev==n && n->prev->next==n);
    }
    for(Node *n=H->prev; n!=H; n=n->prev){
        ccw++;
        assert(n->next && n->prev);
        assert(n->next->prev==n && n->prev->next==n);
    }
    assert(cw==ccw);
    if(cw==0){ assert(H->next==H && H->prev==H); }
    else if(cw==1){ Node *x=H->next; assert(x->next==H && x->prev==H); }
}

/* =============== container_of / entry 辅助 =============== */
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define list_for_each_node(pos, head) \
    for ((pos)=(head)->next; (pos)!=(head); (pos)=(pos)->next)

#define list_for_each_node_safe(pos, n, head) \
    for ((pos)=(head)->next, (n)=(pos)->next; (pos)!=(head); (pos)=(n), (n)=(pos)->next)

/* =============== 统一“基类头” Base（类型标签 + 回调） =============== */
typedef enum { OBJ_USER=1, OBJ_TASK=2, OBJ_FILE=3 } ObjType;

typedef struct Base {
    Node link;                    // 必须放在最前/固定处（约定）
    ObjType type;
    void (*print)(struct Base*);  // 打印/显示
    void (*destroy)(struct Base*);// 释放资源
} Base;

/* =============== 三种外层对象：User / Task / File =============== */
typedef struct User {
    Base base;        // 统一“头”
    int  id;
    char *name;       // 堆分配，演示 destroy
} User;

typedef struct Task {
    Base base;
    int  tid;
    char *title;
    int  priority;
} Task;

typedef struct FileObj {
    Base base;
    char *path;
    size_t size;
} FileObj;

/* ---- 各类型的 print/destroy ---- */
static void user_print(Base *b){
    User *u = container_of(b, User, base);
    printf("[User id=%d name=%s]", u->id, u->name);
}
static void user_destroy(Base *b){
    User *u = container_of(b, User, base);
    free(u->name);
    free(u);
}

static void task_print(Base *b){
    Task *t = container_of(b, Task, base);
    printf("[Task tid=%d pri=%d title=%s]", t->tid, t->priority, t->title);
}
static void task_destroy(Base *b){
    Task *t = container_of(b, Task, base);
    free(t->title);
    free(t);
}

static void file_print(Base *b){
    FileObj *f = container_of(b, FileObj, base);
    printf("[File size=%zu path=%s]", f->size, f->path);
}
static void file_destroy(Base *b){
    FileObj *f = container_of(b, FileObj, base);
    free(f->path);
    free(f);
}

/* ---- 构造器：创建对象并初始化 Base 头 ---- */
static User* user_new(int id, const char *name){
    User *u = (User*)calloc(1, sizeof(*u));       assert(u && "OOM user");
    u->base.link.next = u->base.link.prev = NULL;
    u->base.type = OBJ_USER;
    u->base.print = user_print;
    u->base.destroy = user_destroy;
    u->id = id;
    u->name = (char*)malloc(strlen(name)+1);      assert(u->name && "OOM name");
    strcpy(u->name, name);
    return u;
}
static Task* task_new(int tid, const char *title, int pri){
    Task *t = (Task*)calloc(1, sizeof(*t));       assert(t && "OOM task");
    t->base.link.next = t->base.link.prev = NULL;
    t->base.type = OBJ_TASK;
    t->base.print = task_print;
    t->base.destroy = task_destroy;
    t->tid = tid;
    t->priority = pri;
    t->title = (char*)malloc(strlen(title)+1);    assert(t->title && "OOM title");
    strcpy(t->title, title);
    return t;
}
static FileObj* file_new(const char *path, size_t size){
    FileObj *f = (FileObj*)calloc(1, sizeof(*f)); assert(f && "OOM file");
    f->base.link.next = f->base.link.prev = NULL;
    f->base.type = OBJ_FILE;
    f->base.print = file_print;
    f->base.destroy = file_destroy;
    f->size = size;
    f->path = (char*)malloc(strlen(path)+1);      assert(f->path && "OOM path");
    strcpy(f->path, path);
    return f;
}

/* =============== 通用操作：打印、清理、按谓词删除 =============== */
static void list_print_generic(Node *H, const char *tag){
    printf("%s: ", tag);
    if(list_empty(H)){ puts("[]"); return; }
    Node *p;
    list_for_each_node(p, H){
        Base *b = container_of(p, Base, link);
        b->print(b);
        if(p->next != H) printf(" <-> ");
    }
    puts("");
}

typedef bool (*predicate_fn)(Base *b);

static bool is_user(Base *b){ return b->type==OBJ_USER; }
static bool is_task(Base *b){ return b->type==OBJ_TASK; }
static bool is_file(Base *b){ return b->type==OBJ_FILE; }

static void list_remove_if(Node *H, predicate_fn pred){
    Node *p, *n;
    list_for_each_node_safe(p, n, H){
        Base *b = container_of(p, Base, link);
        if(pred(b)){
            list_erase(p);
            b->destroy(b);
        }
    }
    list_verify(H);
}

static void list_clear(Node *H){
    Node *p, *n;
    list_for_each_node_safe(p, n, H){
        Base *b = container_of(p, Base, link);
        list_erase(p);
        b->destroy(b);
    }
    list_verify(H);
}

/* =============== Demo =============== */
int main(void){
(void)is_file;  // 放在 main() 开头
    Node H; list_init(&H); list_verify(&H);

    /* 在同一条链表上混挂三类对象 */
    User   *u1 = user_new(1, "Tony");
    Task   *t1 = task_new(101, "import-csv", 5);
    FileObj*f1 = file_new("/var/log/syslog", 2048);

    list_insert_before(&H, &u1->base.link); list_verify(&H);
    list_insert_before(&H, &t1->base.link); list_verify(&H);
    list_insert_before(&H, &f1->base.link); list_verify(&H);
    list_print_generic(&H, "After 3 inserts");

    /* 再挂三种：形成更长的混合序列 */
    list_insert_before(&H, &user_new(2,"Bean")->base.link);
    list_insert_before(&H, &task_new(102,"gen-report",3)->base.link);
    list_insert_before(&H, &file_new("/home/tony/note.txt", 1234)->base.link);
    list_verify(&H);
    list_print_generic(&H, "After 6 inserts");

    /* 删除：按类型谓词删除所有 Task */
    list_remove_if(&H, is_task);
    list_print_generic(&H, "Remove all Task");

    /* 删除：按类型谓词删除所有 User */
    list_remove_if(&H, is_user);
    list_print_generic(&H, "Remove all User");

    /* 清空剩余（File） */
    list_clear(&H);
    list_print_generic(&H, "cleanup");
    assert(list_empty(&H));
    return 0;
}
