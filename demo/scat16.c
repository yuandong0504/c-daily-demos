/**SCAT15 — Structured Reaction (SSR)**/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <liburing.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <inttypes.h>

typedef int cap_id_t;
typedef uint64_t msg_id_t;
typedef uint64_t trace_id_t;

static unsigned long g_msg_created  = 0;
static unsigned long g_msg_handled  = 0;
static unsigned long g_msg_dropped  = 0;

static struct io_uring g_ring;
static char g_stdin_buf[1024];

static int g_running=1;
static int g_edge_count=0;
static trace_id_t g_trace_max = 0;

#define USE_CURRENT_OFFSET (-1)
#define FILE_OFFSET_START (0)
#define NO_FLAGS (0)
#define MAX_EDGES 1024
#define MAX_STEPS 8
#define SIGQ_CAP 64

// ===== CAP (C) — resource-carrying capability =====
typedef uint32_t cap_rights_t;
enum { CAP_NONE = 0 };
enum { CAP_TABLE_SIZE = 64 };
enum {
    CAP_R_READ  = 1u << 0,
    CAP_R_WRITE = 1u << 1,
};

typedef struct {
    int fd;                // anchor
    cap_rights_t rights;   // permissions
    int in_use;             // 0/1
} CapEntry;

static CapEntry g_caps[CAP_TABLE_SIZE];

static void cap_init(void)
{
    for (int i = 0; i < CAP_TABLE_SIZE; i++) {
        g_caps[i].fd = -1;
        g_caps[i].rights = 0;
        g_caps[i].in_use = 0;
    }
}

static int cap_is_valid(cap_id_t cap)
{
    if (cap == CAP_NONE) return 0;  
    if (cap < 0) return 0;
    if (cap >= CAP_TABLE_SIZE) return 0;
    return g_caps[cap].in_use == 1;
}

static cap_id_t mint_cap(int fd, cap_rights_t rights)
{
    if (fd < 0) return CAP_NONE;
    for (int i = 1; i < CAP_TABLE_SIZE; i++) {
        if (!g_caps[i].in_use) {
            g_caps[i].fd = fd;
            g_caps[i].rights = rights;
            g_caps[i].in_use = 1;
            return (cap_id_t)i;
        }
    }
    return CAP_NONE;
}

static void cap_revoke(cap_id_t cap)
{
    if (cap <= 0 || cap >= CAP_TABLE_SIZE) return;
    g_caps[cap].in_use = 0;
    g_caps[cap].fd = -1;
    g_caps[cap].rights = 0;
}
static int cap_write_all(cap_id_t cap, const void *buf, size_t len)
{
    if (!cap_is_valid(cap)) return -1;
    if ((g_caps[cap].rights & CAP_R_WRITE) == 0) return -2;

    int fd = g_caps[cap].fd;

    const char *p = (const char *)buf;
    size_t left = len;
    while (left > 0) {
        ssize_t n = write(fd, p, left);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -3;
        }
        p += (size_t)n;
        left -= (size_t)n;
    }
    return 0;
}
typedef enum{
    WORK_OK=0,
    WORK_ERROR=1,
}WorkResult;
typedef enum{
    TARGET_A,
    TARGET_B,
    TARGET_BOTH
}Target;
typedef enum{
    MSGK_APP,
    MSGK_STDIN_LINE
}MessageKind;
typedef struct{
    Target target;
    cap_id_t cap;
}Step;
typedef struct{
    Step steps[MAX_STEPS];
    int count;
}TargetSteps;
typedef struct{
    msg_id_t id;
    msg_id_t parent_msg_id;
    cap_id_t cap;
    trace_id_t trace_id;
    MessageKind kind;
    Target to;
    char *payload;
    const TargetSteps *steps;
    int step_idx;
}Message;
typedef struct {
    msg_id_t   msg_id;
    trace_id_t trace_id;
    msg_id_t   parent_msg_id;
    const TargetSteps *steps;
    int step_idx;
    MessageKind kind;
    char *payload;

    WorkResult result;
} WorkSignal;
typedef struct{
    trace_id_t trace_id;
    msg_id_t msg_id;
    msg_id_t parent_msg_id;
    const char *from;
    const char *to;
}MsgEdge;
static MsgEdge g_edges[MAX_EDGES];

static WorkSignal g_sigq[SIGQ_CAP];
static int g_sig_h=0,g_sig_t=0;
static int sigq_empty(void) { return g_sig_h == g_sig_t; }
static int sigq_full(void)  { return ((g_sig_t + 1) % SIGQ_CAP) == g_sig_h; }
static int sigq_push(const WorkSignal *s)
{
    if (sigq_full()) return -1;
    g_sigq[g_sig_t] = *s;
    g_sig_t = (g_sig_t + 1) % SIGQ_CAP;
    return 0;
}
static int sigq_pop(WorkSignal *out)
{
    if (sigq_empty()) return -1;
    *out = g_sigq[g_sig_h];
    g_sig_h = (g_sig_h + 1) % SIGQ_CAP;
    return 0;
}
static void signal_emit(const Message *m, WorkResult r)
{
    WorkSignal s = {
        .msg_id   = m->id,
        .trace_id = m->trace_id,
        .kind     = m->kind,
        .payload  = m->payload,
        .steps    = m->steps,
        .step_idx = m->step_idx,
        .result   = r,
    };

    if (sigq_push(&s) != 0)
    {
        fprintf(stderr, "[SIGQ_FULL] drop signal for msg=%"PRIu64"\n", m->id);
    }
}
enum {TRACE_NONE=0};


static trace_id_t mint_trace(void)
{
    static trace_id_t mint_trace_id=0;
    trace_id_t t = ++mint_trace_id;
    g_trace_max = t;
    return t;
}
static void mint_message(Message *m)
{
    static msg_id_t mint_msg_id=0;

    if(m->trace_id==TRACE_NONE)
    {
        m->trace_id=mint_trace();
    }
    m->id=++mint_msg_id;
}
static void record_edge(trace_id_t trace_id,
                        msg_id_t msg_id,
                        msg_id_t parent_msg_id,
                        const char *from,
                        const char *to)
{
    if(g_edge_count>=MAX_EDGES)return;
    g_edges[g_edge_count++]=(MsgEdge){
        .trace_id=trace_id,
            .msg_id=msg_id,
            .parent_msg_id=parent_msg_id,
            .from=from,
            .to=to
    };
}
#define INBOX_CAP 16
typedef struct{
    Message msgs[INBOX_CAP];
    int head;
    int tail;
}Inbox;
static void inbox_init(Inbox *q)
{
    q->head=q->tail=0;
}
static int inbox_empty(Inbox *q)
{
    return q->head==q->tail;
}
static int inbox_full(Inbox *q)
{
    return ((q->tail+1)%INBOX_CAP)==q->head;
}
static int inbox_push(Inbox *q,const Message *m)
{
    if(inbox_full(q)) return -1;
    q->msgs[q->tail]=*m;
    q->tail=((q->tail+1)%INBOX_CAP);
    return 0;
}
static int inbox_pop(Inbox *q,Message *out)
{
    if(inbox_empty(q)) return -1;
    *out=q->msgs[q->head];
    q->head=((q->head+1)%INBOX_CAP);
    return 0;
}
typedef struct Doer Doer;
struct Doer{
    const char *name;
    Inbox inbox;
    void (*handle)(Doer *self,const Message *msg);
};
static void doer_a_handle(Doer *self,const Message *msg)
{
    (void)self;
    if(msg->kind==MSGK_STDIN_LINE)
    {
        printf("[A]:message from stdin\n");
    }
    printf("msg %"PRIu64" (p %"PRIu64") cap %d [A]:%s\n",msg->id,msg->parent_msg_id,msg->cap,msg->payload);
    signal_emit(msg, WORK_OK);
}
static void doer_b_handle(Doer *self,const Message * msg)
{
    if(msg->kind==MSGK_STDIN_LINE)
    {
        printf("[B]:message from stdin\n");
    }
    (void)self;
    printf("msg %"PRIu64" (p %"PRIu64") cap %d [B]:%s\n",msg->id,msg->parent_msg_id,msg->cap,msg->payload);
    signal_emit(msg, WORK_OK);
}

static Doer g_doer_a;
static Doer g_doer_b;
static const TargetSteps STEP_STDIN_A_THEN_B={
    .steps={
        {.target=TARGET_A,.cap=0},
        {.target=TARGET_B,.cap=0},
    },
    .count=2
}; 
static void runtime_init(void)
{
    g_doer_a.name="A";
    g_doer_a.handle=doer_a_handle;
    inbox_init(&g_doer_a.inbox);

    g_doer_b.name="B";
    g_doer_b.handle=doer_b_handle;
    inbox_init(&g_doer_b.inbox);
}

static void runtime_record_drop(const Message *m, Doer *d)
{
    record_edge(m->trace_id,m->id,m->parent_msg_id,d->name,"dropped");
    g_msg_dropped++;
    printf(
    "[DROP] msg=%"PRIu64" (p %"PRIu64") cap=%d to=%s payload=\"%s\"\n",
    m->id,
    m->parent_msg_id,
    m->cap,
    d->name,
    m->payload ? m->payload : "");
}
// === RUNTIME ===
// Does NOT perform permission checks.
static void runtime_emit(const Message *src,Doer *d)
{
    Message m=*src;
    g_msg_created++;
    record_edge(m.trace_id,m.id,m.parent_msg_id,"runtime",d->name);
    if (inbox_push(&d->inbox, &m) != 0)
    {
        runtime_record_drop(&m, d);
    }
}
// === RUNTIME ===
// Executes already-validated actions.
// Does NOT perform permission checks.
static void runtime_route(const Message *msg)
{
    switch(msg->to)
    {
        case TARGET_A:
            runtime_emit(msg,&g_doer_a);
            break;
        case TARGET_B:
            runtime_emit(msg,&g_doer_b);
            break;
        case TARGET_BOTH:
            fprintf(stderr, "[BUG] TARGET_BOTH must not enter runtime; producer must fan-out\n");
            break;
    }
}
static void dispatch_doer(Doer *d)
{
    Message msg;
    while(inbox_pop(&d->inbox,&msg)==0)
    {
        d->handle(d,&msg);
    }
}
#define MAX_DOERS 8
typedef struct{
    Doer *list[MAX_DOERS];
    int count;
}DoerRegistry;
static void registry_init(DoerRegistry *r)
{
    r->count=0;
}
static int registry_add(DoerRegistry *r,Doer *d)
{
    if(r->count>=MAX_DOERS) return -1;
    r->list[r->count++]=d;
    return 0;
}
typedef struct{
    DoerRegistry *reg;
}Scheduler;

static int validate_steps(const TargetSteps *s, const DoerRegistry *reg)
{
    (void)reg;
    if (!s) return -1;
    if (s->count <=0||s->count>MAX_STEPS) return -1;

    for (int i = 0; i < s->count; i++) {
        // target 必须是已知枚举
        if (s->steps[i].target != TARGET_A &&
            s->steps[i].target != TARGET_B) {
            return -1;
        }
        // cap 必须是一个“显式 token”
        if (s->steps[i].cap < 0) return -1;
    }
    return 0;
}

static const TargetSteps* assemble_steps(const TargetSteps *s, const DoerRegistry *reg)
{
    if (validate_steps(s, reg) != 0) {
        fprintf(stderr, "[STEPS_INVALID]\n");
        return NULL;
    }
    return s;
}
static int scheduler_has_work(const Scheduler *s)
{
    for(int i=0;i<s->reg->count;i++)
    {
        Doer *d=s->reg->list[i];
        if(!inbox_empty(&d->inbox)){return 1;}
    }
    return 0;
}
static unsigned long runtime_pending_messages(const DoerRegistry *reg)
{
    unsigned long n = 0;
    for (int i = 0; i < reg->count; i++) {
        Inbox *q = &reg->list[i]->inbox;
        if (q->tail >= q->head)
            n += (q->tail - q->head);
        else
            n += (INBOX_CAP - q->head + q->tail);
    }
    return n;
}
static void runtime_print_message_balance(const DoerRegistry *reg)
{
    unsigned long pending = runtime_pending_messages(reg);
    long balance = (long)g_msg_created
                 - (long)g_msg_handled
                 - (long)g_msg_dropped
                 - (long)pending;
    printf("[MSG_BALANCE] created=%lu handled=%lu dropped=%lu pending=%lu balance=%ld\n",
       g_msg_created, g_msg_handled, g_msg_dropped,pending, balance);
}
// === RUNTIME ===
// Does NOT perform permission checks.
static void scheduler_round(Scheduler *s)
{
    for(int i=0;i<s->reg->count;i++)
    {
        Doer *d=s->reg->list[i];
        Message m;
        if(inbox_pop(&d->inbox,&m)==0)
        {
            record_edge(m.trace_id,
                        m.id,
                        m.parent_msg_id,
                        d->name,
                        "handled");
            d->handle(d,&m);
            g_msg_handled++;
        }
    }
}
// External world → CMR boundary
// Raw events must be converted into Messages before entering runtime.
static void emit_stdin_line_message(char *line)
{
    const TargetSteps *steps =assemble_steps(&STEP_STDIN_A_THEN_B,NULL);
    if(!steps)return;

    char *p=line;
    while(*p==' '||*p=='\t') p++;
    if(*p=='\0') return;
    Message msg={
        .parent_msg_id=0,
        .trace_id=TRACE_NONE,
        .kind=MSGK_STDIN_LINE,
        .payload=p,

        .steps=steps,
        .step_idx=0,
        .to=steps->steps[0].target,
        .cap=steps->steps[0].cap,

    };
    mint_message(&msg);
    record_edge(msg.trace_id,msg.id,msg.parent_msg_id,"WORLD","stdin");
    runtime_route(&msg);
}
static void emit_stdin_event(void)
{
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;

    sqe=io_uring_get_sqe(&g_ring);
    io_uring_prep_read(sqe,STDIN_FILENO,g_stdin_buf,sizeof(g_stdin_buf)-1,FILE_OFFSET_START);
    int sret = io_uring_submit(&g_ring);
    if (sret < 0) 
    { 
        fprintf(stderr, "submit: %s\n", strerror(-sret)); 
        g_running=0; 
        return; 
    }
    int ret=io_uring_wait_cqe(&g_ring,&cqe);
    if(ret==-EINTR){return;}
    if(ret<0)
    {
        fprintf(stderr,"io_uring_wait_cqe failed:%s\n",strerror(-ret));
        g_running=0;
        return;
    }
    int res=cqe->res;
    io_uring_cqe_seen(&g_ring,cqe);
    if(res==0)
    {
        printf("EOF,exit.\n");
        g_running=0;
        return;
    }
    if(res<0)
    {
        fprintf(stderr,"read failed:%s\n",strerror(-res));
        g_running=0;
        return;
    }
    g_stdin_buf[res] = '\0';
    if(res>0&&g_stdin_buf[res-1]=='\n')
    {
        g_stdin_buf[res-1]='\0';
    }
    emit_stdin_line_message(g_stdin_buf);
}
static void on_sigint(int signo)
{
    (void)signo;
    g_running=0;
}
static void dump_trace(trace_id_t trace_id)
{
    for(int i=0;i<g_edge_count;i++)
    {
        if(g_edges[i].trace_id==trace_id)
        {
            printf("msg %"PRIu64" (p %"PRIu64") %s->%s\n",
                   g_edges[i].msg_id,
                   g_edges[i].parent_msg_id,
                   g_edges[i].from,
                   g_edges[i].to);
        }
    }
}
static void reactor_run(void)
{
    WorkSignal s;

    while (sigq_pop(&s) == 0)
    {
       if (s.result != WORK_OK)
            continue;

        int next = s.step_idx + 1;
        if (!s.steps || next >= s.steps->count)
            continue;

        Message next_msg = {
            .parent_msg_id = s.msg_id,
            .trace_id      = s.trace_id,
            .kind          = s.kind,
            .payload       = s.payload,

            .steps    = s.steps,
            .step_idx = next,
            .to       = s.steps->steps[next].target,
            .cap      = s.steps->steps[next].cap,
        };

        mint_message(&next_msg);
        runtime_route(&next_msg);
    }
}
int main(void)
{
    printf("=== SCAT15: Message Structured Reaction (SSR) ===\n");
    signal(SIGINT,on_sigint);

    int ret;
    ret=io_uring_queue_init(8,&g_ring,NO_FLAGS);
    if(ret<0)
    {
        fprintf(stderr,"io_uring_queue_init failed:%s\n",strerror(-ret));
        return 1;  
    }

    runtime_init();
    DoerRegistry reg;
    registry_init(&reg);
    registry_add(&reg,&g_doer_a);
    registry_add(&reg,&g_doer_b);

    trace_id_t tid = mint_trace();
    /* to A */
    Message mA = {.parent_msg_id = 0,.to=TARGET_A, .trace_id=tid, .cap=1, .payload="hi Tony." };
    mint_message(&mA);
    runtime_route(&mA);
    /* to B */
    Message mB = {.parent_msg_id = 0,.to=TARGET_B, .trace_id=tid, .cap=1, .payload="hi Tony." };
    mint_message(&mB);
    runtime_route(&mB);

    Message m1={.parent_msg_id = 0,.to=TARGET_A,.trace_id=TRACE_NONE,.cap=1,.payload="hi Bean."};
    mint_message(&m1);
    runtime_route(&m1);

    Message m2={.parent_msg_id = 0,.to=TARGET_B,.trace_id=TRACE_NONE,.cap=2,.payload="hi Alex."};
    mint_message(&m2);
    runtime_route(&m2);

    tid=mint_trace();
    /* to A */
    Message mA1 = {.parent_msg_id = 0,.to=TARGET_A, .trace_id=tid, .cap=2, .payload="yea all" };
    mint_message(&mA1);
    runtime_route(&mA1);
    
    /* to B */
    Message mB1={.parent_msg_id = 0,.to=TARGET_B,.trace_id=tid,.cap=2,.payload="yea all"};
    mint_message(&mB1);
    runtime_route(&mB1);

    Scheduler sched={.reg=&reg};
    while(g_running)
    {
        emit_stdin_event();
        while(scheduler_has_work(&sched)||!sigq_empty())
        {
            while (scheduler_has_work(&sched)) 
            {
                scheduler_round(&sched);
            }
            reactor_run();
        }
        runtime_print_message_balance(&reg);
    }
    
    io_uring_queue_exit(&g_ring);
    printf("io_uring is cleaned.\n");
    for(int i=0;i<g_trace_max;i++)
    {
        printf("\n");
        dump_trace(i+1);
    }
    return 0;
}
