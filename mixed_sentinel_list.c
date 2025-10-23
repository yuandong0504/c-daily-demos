// gcc -Wall -Wextra -O2 mixed_sentinel_list_v2.c -o mixed_list && ./mixed_list
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>

#define BASE_MAGIC 0xBACEBACEU
/*================ 基础双向循环链表（哨兵） ================*/
typedef struct Node { struct Node *prev, *next; } Node;

static inline void list_init(Node *H){ H->next=H; H->prev=H; }
static inline bool list_empty(Node *H){ return H->next==H; }

static inline void list_insert_after(Node *pos, Node *x){
    assert(pos && x);
    assert(x->next==NULL && x->prev==NULL); // 游离态
    x->next = pos->next;
    x->prev = pos;
    pos->next->prev = x;
    pos->next = x;
}
static inline void list_insert_before(Node *pos, Node *x){
    list_insert_after(pos->prev, x);
}

/* 调试态：抹毒，便于定位二次使用/双删 */
#ifndef NDEBUG
#  define POISON ((void*)0xDEADBEEF)
#else
#  define POISON ((void*)0)
#endif

static inline void list_erase(Node *x){
    assert(x && x->next && x->prev);
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x->prev = POISON; // 置游离/抹毒
}

/*=============== 自证：更强健（步数上限 + 快慢指针） ===============*/
static void list_verify_bound(Node *H, size_t max_nodes){
    assert(H && H->next && H->prev);
    assert(H->next->prev==H && H->prev->next==H);

    /* 快慢指针：排除“脱离哨兵的坏环” */
    Node *slow = H->next, *fast = H->next;
    while (fast!=H && fast->next!=H) {
        slow = slow->next;
        fast = fast->next->next;
        assert(slow != fast && "cycle without sentinel");
    }

    size_t cw=0, ccw=0, steps=0;

    for(Node *n=H->next; n!=H; n=n->next){
        assert(++steps <= max_nodes && "verify bound exceeded");
        cw++;
        assert(n->next && n->prev);
        assert(n->next->prev==n && n->prev->next==n);
    }
    steps=0;
    for(Node *n=H->prev; n!=H; n=n->prev){
        assert(++steps <= max_nodes && "verify bound exceeded");
        ccw++;
        assert(n->next && n->prev);
        assert(n->next->prev==n && n->prev->next==n);
    }
    assert(cw==ccw);
    if(cw==0){ assert(H->next==H && H->prev==H); }
    else if(cw==1){ Node *x=H->next; assert(x->next==H && x->prev==H); }
}
static inline void list_verify(Node *H){ list_verify_bound(H, 1000000u); }

/*================ container_of / 遍历辅助 ================*/
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define list_for_each_node(pos, head) \
    for ((pos)=(head)->next; (pos)!=(head); (pos)=(pos)->next)

#define list_for_each_node_safe(pos, n, head) \
    for ((pos)=(head)->next, (n)=(pos)->next; (pos)!=(head); (pos)=(n), (n)=(pos)->next)

/* 容器语义别名（读起来更像“队列/双端队列”） */
static inline void list_push_back(Node *H, Node *x){ list_insert_before(H, x); }
static inline void list_push_front(Node *H, Node *x){ list_insert_after(H, x); }

/*================ 统一“基座头” Base（magic + 类型 + 回调） ================*/
typedef enum { OBJ_USER=1, OBJ_TASK=2, OBJ_FILE=3 } ObjType;
typedef struct Base {
    unsigned int magic;                     // ✅ 身份签名
    Node link;                     // 固定偏移，用于 container_of
    ObjType type;
    void (*print)(struct Base*);
    void (*destroy)(struct Base*);
} Base;

/*================ 三类对象：User / Task / File ================*/
typedef struct User  { Base base;  int id;  char *name;         } User;
typedef struct Task  { Base base;  int tid; char *title; int priority; } Task;
typedef struct FileObj { Base base; char *path; size_t size;    } FileObj;

/*---- 各类型 print/destroy ----*/
static void user_print(Base *b){
    assert(b && b->magic==BASE_MAGIC);
    User *u = container_of(b, User, base);
    printf("[User id=%d name=%s]", u->id, u->name);
}
static void user_destroy(Base *b){
    if(!b) return;
    assert(b->magic==BASE_MAGIC);
    User *u = container_of(b, User, base);
    b->magic = 0;                // 退役
    free(u->name);
    free(u);
}

static void task_print(Base *b){
    assert(b && b->magic==BASE_MAGIC);
    Task *t = container_of(b, Task, base);
    printf("[Task tid=%d pri=%d title=%s]", t->tid, t->priority, t->title);
}
static void task_destroy(Base *b){
    if(!b) return;
    assert(b->magic==BASE_MAGIC);
    Task *t = container_of(b, Task, base);
    b->magic = 0;
    free(t->title);
    free(t);
}

static void file_print(Base *b){
    assert(b && b->magic==BASE_MAGIC);
    FileObj *f = container_of(b, FileObj, base);
    printf("[File size=%zu path=%s]", f->size, f->path);
}
static void file_destroy(Base *b){
    if(!b) return;
    assert(b->magic==BASE_MAGIC);
    FileObj *f = container_of(b, FileObj, base);
    b->magic = 0;
    free(f->path);
    free(f);
}

/*---- 构造器：用 strdup 简化 ----*/
static User* user_new(int id, const char *name){
    User *u = (User*)calloc(1, sizeof(*u));       assert(u && "OOM user");
    u->base.magic   = BASE_MAGIC;
    u->base.link.next = u->base.link.prev = NULL;
    u->base.type    = OBJ_USER;
    u->base.print   = user_print;
    u->base.destroy = user_destroy;
    u->id = id;
    u->name = strdup(name);                       assert(u->name && "OOM name");
    return u;
}
static Task* task_new(int tid, const char *title, int pri){
    Task *t = (Task*)calloc(1, sizeof(*t));       assert(t && "OOM task");
    t->base.magic   = BASE_MAGIC;
    t->base.link.next = t->base.link.prev = NULL;
    t->base.type    = OBJ_TASK;
    t->base.print   = task_print;
    t->base.destroy = task_destroy;
    t->tid = tid; t->priority = pri;
    t->title = strdup(title);                    assert(t->title && "OOM title");
    return t;
}
static FileObj* file_new(const char *path, size_t size){
    FileObj *f = (FileObj*)calloc(1, sizeof(*f)); assert(f && "OOM file");
    f->base.magic   = BASE_MAGIC;
    f->base.link.next = f->base.link.prev = NULL;
    f->base.type    = OBJ_FILE;
    f->base.print   = file_print;
    f->base.destroy = file_destroy;
    f->size = size;
    f->path = strdup(path);                      assert(f->path && "OOM path");
    return f;
}

/*================ 通用操作：打印、按谓词删除（带上下文）、清空 ================*/
static void list_print_generic(Node *H, const char *tag){
    printf("%s: ", tag);
    if(list_empty(H)){ puts("[]"); return; }
    Node *p;
    list_for_each_node(p, H){
        Base *b = container_of(p, Base, link);
        assert(b->magic==BASE_MAGIC);
        b->print(b);
        if(p->next != H) printf(" <-> ");
    }
    puts("");
}

/* 谓词带上下文：更灵活 */
typedef bool (*predicate_fn)(Base *b, void *ctx);
static bool pred_is_type(Base *b, void *ctx){
    return b->type == *(const ObjType*)ctx;
}

/* 删除返回计数，便于测试与组合 */
static size_t list_remove_if(Node *H, predicate_fn pred, void *ctx){
    size_t removed = 0;
    Node *p, *n;
    list_for_each_node_safe(p, n, H){
        Base *b = container_of(p, Base, link);
        assert(b->magic==BASE_MAGIC);
        if(pred(b, ctx)){
            list_erase(p);
            b->destroy(b);
            ++removed;
        }
    }
    list_verify(H);
    return removed;
}

static size_t list_clear(Node *H){
    size_t cleared = 0;
    Node *p, *n;
    list_for_each_node_safe(p, n, H){
        Base *b = container_of(p, Base, link);
        assert(b->magic==BASE_MAGIC);
        list_erase(p);
        b->destroy(b);
        ++cleared;
    }
    list_verify(H);
    return cleared;
}

/*================ Demo ================*/
int main(void){
    Node H; list_init(&H); list_verify(&H);

    /* 混挂三类对象 */
    User   *u1 = user_new(1, "Tony");
    Task   *t1 = task_new(101, "import-csv", 5);
    FileObj*f1 = file_new("/var/log/syslog", 2048);

    list_push_back(&H, &u1->base.link); list_verify(&H);
    list_push_back(&H, &t1->base.link); list_verify(&H);
    list_push_back(&H, &f1->base.link); list_verify(&H);
    list_print_generic(&H, "After 3 inserts");

    /* 再挂三种 */
    list_push_back(&H, &user_new(2,"Bean")->base.link);
    list_push_back(&H, &task_new(102,"gen-report",3)->base.link);
    list_push_back(&H, &file_new("/home/tony/note.txt", 1234)->base.link);
    list_verify(&H);
    list_print_generic(&H, "After 6 inserts");

    /* 删除：按类型谓词 */
    ObjType T=OBJ_TASK, U=OBJ_USER;
    size_t n1 = list_remove_if(&H, pred_is_type, &T);
    list_print_generic(&H, "Remove all Task");
    size_t n2 = list_remove_if(&H, pred_is_type, &U);
    list_print_generic(&H, "Remove all User");

    /* 清空剩余（File） */
    size_t n3 = list_clear(&H);
    list_print_generic(&H, "cleanup");
    assert(list_empty(&H));

    fprintf(stderr, "removed: Task=%zu, User=%zu, File=%zu\n", n1, n2, n3);
    return 0;
}
