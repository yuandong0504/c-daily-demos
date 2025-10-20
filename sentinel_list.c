// sentinel_list.c  —  Circular doubly-linked list with sentinel + verify
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct Node {
    struct Node *next, *prev;
    int val;               // 演示用负载（哨兵可不使用）
    int is_sentinel;       // 标记哨兵节点（便于打印/校验可读）
} Node;

/* ========== 基本形态 ========== */
void list_init(Node *H) {
    H->next = H;
    H->prev = H;
    H->is_sentinel = 1;
}

/* 空否：哨兵自指等价于空表 */
int list_empty(Node *H) { return H->next == H; }

/* ========== 原子操作（统一 4 指针，无特判） ========== */
void insert_after(Node *pos, Node *x) {
    x->next = pos->next;
    x->prev = pos;
    pos->next->prev = x;
    pos->next = x;
}

void insert_before(Node *pos, Node *x) {
    insert_after(pos->prev, x);
}

/* 从环上摘掉 x（不 free） */
void erase(Node *x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    // 调试友好：清毒值（可注释）
    x->next = x->prev = NULL;
}

/* ========== 打印（从 H->next 起一圈） ========== */
void list_print(Node *H, const char *tag) {
    printf("%s: ", tag);
    Node *n = H->next;
    if (n == H) { printf("[ ]\n"); return; }
    while (n != H) {
        printf("[%d]", n->val);
        n = n->next;
        if (n != H) printf(" <-> ");
    }
    printf("\n");
}

/* ========== 轻量验证（顺/逆一圈 + 对称 + 计数一致 + 上限） ========== */
int list_verify(Node *H, size_t max_nodes) {
    // 0) 哨兵自洽
    assert(H->next != NULL && H->prev != NULL);

    // 1) 顺时针一圈：局部对称 + 上限
    size_t cnt_cw = 0;
    Node *n = H->next;
    while (n != H) {
        // 局部对称：prev->next == n && next->prev == n
        assert(n->prev != NULL && n->prev->next == n);
        assert(n->next != NULL && n->next->prev == n);

        cnt_cw++;
        assert(cnt_cw <= max_nodes);  // 防多环/死圈
        n = n->next;
    }

    // 2) 逆时针一圈：局部对称 + 上限
    size_t cnt_ccw = 0;
    n = H->prev;
    while (n != H) {
        assert(n->next != NULL && n->next->prev == n);
        assert(n->prev != NULL && n->prev->next == n);

        cnt_ccw++;
        assert(cnt_ccw <= max_nodes);
        n = n->prev;
    }

    // 3) 收口：顺逆计数一致（结构完整、无分叉-合流怪图）
    assert(cnt_cw == cnt_ccw);

    // 4) 可选：空表/单节点直觉性检查
    if (cnt_cw == 0) {
        assert(H->next == H && H->prev == H);
    } else if (cnt_cw == 1) {
        Node *x = H->next;
        assert(x->next == H && x->prev == H);
    }

    return 1;
}

/* ========== 演示用例 ========== */
int main(void) {
    Node H; list_init(&H);

    // 台阶 0：空环
    list_verify(&H, 1000);
    list_print(&H, "init");

    // 台阶 1：插入 1 个
    Node *a = (Node*)calloc(1, sizeof(Node)); a->val = 1;
    insert_after(&H, a);
    list_verify(&H, 1000);
    list_print(&H, "after insert 1");

    // 台阶 2：再插入到“尾前”（等价 append）
    Node *b = (Node*)calloc(1, sizeof(Node)); b->val = 2;
    insert_before(&H, b);   // [1] <-> [2]
    Node *c = (Node*)calloc(1, sizeof(Node)); c->val = 3;
    insert_before(&H, c);   // [1] <-> [2] <-> [3]
    list_verify(&H, 1000);
    list_print(&H, "after insert 2,3");

    // 删除中间
    erase(b); free(b);
    list_verify(&H, 1000);
    list_print(&H, "erase 2");

    // 删除首尾
    erase(a); free(a);
    list_verify(&H, 1000);
    list_print(&H, "erase 1");
    erase(c); free(c);
    list_verify(&H, 1000);
    list_print(&H, "erase 3 -> empty");

    // 再插几下试试稳定性
    for (int i = 1; i <= 5; ++i) {
        Node *x = (Node*)calloc(1, sizeof(Node)); x->val = i*10;
        insert_after(&H, x);
        list_verify(&H, 1000);
    }
    list_print(&H, "after insert 10..50");

    // 回收（演示：从尾起依次删）
    while (!list_empty(&H)) {
        Node *x = H.prev;      // 尾
        erase(x);
        free(x);
        list_verify(&H, 1000);
    }
    list_print(&H, "cleanup");

    return 0;
}
