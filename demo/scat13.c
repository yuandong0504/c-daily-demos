/*
 * SCAT13 — Message Topology Exposure
 *
 * Status: COMPLETE
 *
 * Scope:
 *   - expose runtime message flow as edges
 *
 * Non-goals:
 *   - explanation
 *   - causality
 *   - authority
 *   - responsibility
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <liburing.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <inttypes.h>

typedef uint64_t cap_id_t;
typedef uint64_t msg_id_t;
typedef uint64_t trace_id_t;

static unsigned long g_msg_created  = 0;
static unsigned long g_msg_handled  = 0;
static unsigned long g_msg_dropped  = 0;

static struct io_uring g_ring;
static char g_stdin_buf[1024];

static int g_running=1;
static int g_edge_count=0;
// === MINT ===
// Responsible for creating unique message/capability identities.
static msg_id_t mint_msg_id=0;
static cap_id_t mint_cap_id=0;
static trace_id_t mint_trace_id=0;

#define USE_CURRENT_OFFSET (-1)
#define FILE_OFFSET_START (0)
#define NO_FLAGS (0)
#define MAX_EDGES 1024

typedef enum{
    CMD_SEND_A,
    CMD_SEND_B,
    CMD_SEND_BOTH,
    CMD_EXIT,
    CMD_UNKNOWN
}CommandType;
typedef struct Command{
    CommandType type;
    char *text;
}Command;
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
    msg_id_t id;
    cap_id_t cap;
    trace_id_t trace_id;
    MessageKind kind;
    Target to;
    char *payload;
}Message;
typedef struct{
    trace_id_t trace_id;
    msg_id_t msg_id;
    const char *from;
    const char *to;
}MsgEdge;
static MsgEdge g_edges[MAX_EDGES];
typedef struct{
    cap_id_t allowed_caps[4];
    int cap_count;
}CapabilitySet;
static void record_edge(trace_id_t trace_id,
                        msg_id_t msg_id,
                        const char *from,
                        const char *to)
{
    if(g_edge_count>=MAX_EDGES)return;
    g_edges[g_edge_count++]=(MsgEdge){
        .trace_id=trace_id,
            .msg_id=msg_id,
            .from=from,
            .to=to
    };
}
static Command parse_command(char *line)
{
    Command cmd={.type=CMD_UNKNOWN,.text=NULL};
    while(*line==' '||*line=='\t'){line++;}
    if(line[0]=='\0')return cmd;
    if(strcmp(line,"exit")==0)
    {
        cmd.type=CMD_EXIT;
        return cmd;
    }
    if(strncmp(line,"a ",2)==0)
    {
        cmd.type=CMD_SEND_A;
        cmd.text=line+2;
        return cmd;
    }
    if(strncmp(line,"b ",2)==0)
    {
        cmd.type=CMD_SEND_B;
        cmd.text=line+2;
        return cmd;
    }
    if(strncmp(line,"both ",5)==0)
    {
        cmd.type=CMD_SEND_BOTH;
        cmd.text=line+5;
        return cmd;
    }
    return cmd;
}
typedef enum{
    C2M_OK,
    C2M_CTRL_EXIT,
    C2M_NOOP
}C2MResult;
C2MResult command_to_message(Command *cmd,Message *msg)
{
    switch(cmd->type){
        case CMD_SEND_A:
            msg->to=TARGET_A;
            msg->payload=cmd->text;
            //g_msg_created++;
            return C2M_OK;
        case CMD_SEND_B:
            msg->to=TARGET_B;
            msg->payload=cmd->text;
            //g_msg_created++;
            return C2M_OK;
        case CMD_SEND_BOTH:
            msg->to=TARGET_BOTH;
            msg->payload=cmd->text;
            //g_msg_created++;
            return C2M_OK;
        case CMD_EXIT:
            return C2M_CTRL_EXIT;
        case CMD_UNKNOWN:
        default:
            return C2M_NOOP;
    }
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
    CapabilitySet caps;
    void (*handle)(Doer *self,const Message *msg);
};
static void doer_a_handle(Doer *self,const Message *msg)
{
    (void)self;
    if(msg->kind==MSGK_STDIN_LINE)
    {
        printf("[A]:message from stdin\n");
    }
    printf("msg %"PRIu64" cap %"PRIu64" [A]:%s\n",msg->id,msg->cap,msg->payload);
}
static void doer_b_handle(Doer *self,const Message *  msg)
{
    (void)self;
    printf("msg %"PRIu64" cap %"PRIu64" [B]:%s\n",msg->id,msg->cap,msg->payload);
}
static Doer g_doer_a;
static Doer g_doer_b;
static void runtime_init(void)
{
    g_doer_a.name="A";
    g_doer_a.handle=doer_a_handle;
    inbox_init(&g_doer_a.inbox);
    g_doer_a.caps.allowed_caps[0]=1;
    g_doer_a.caps.cap_count=1;

    g_doer_b.name="B";
    g_doer_b.handle=doer_b_handle;
    inbox_init(&g_doer_b.inbox);
    g_doer_b.caps.allowed_caps[0]=2;
    g_doer_b.caps.cap_count=1;
}
// === MINT ===
// Responsible for creating unique message/capability identities.
static cap_id_t mint_new_cap(void)
{
    return ++mint_cap_id;
}
// === VALIDATE ===
// Determines whether a minted capability is usable by a given doer.
// Returns boolean only. No side effects.
static int validate_capability(cap_id_t cap,const CapabilitySet *set)
{
    for(int i=0;i<set->cap_count;i++)
    {
        if(set->allowed_caps[i]==cap) 
            return 1;
    }
    return 0;
}
static void runtime_record_drop(const Message *m, Doer *d)
{
    record_edge(m->trace_id,m->id,d->name,"dropped");
    g_msg_dropped++;
    printf(
    "[DROP] msg=%"PRIu64" cap=%"PRIu64" to=%s payload=\"%s\"\n",
    m->id,
    m->cap,
    d->name,
    m->payload ? m->payload : "");
}
// === RUNTIME ===
// Executes already-validated actions.
// Does NOT perform permission checks.
static void runtime_emit(const Message *src,Doer *d)
{
    Message m=*src;
    g_msg_created++;
    m.id=++mint_msg_id;
    record_edge(m.trace_id,m.id,"runtime",d->name);
    if(!validate_capability(m.cap,&d->caps))
    {
        runtime_record_drop(&m, d);
    }
    else if (inbox_push(&d->inbox, &m) != 0)
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
            runtime_emit(msg,&g_doer_a);
            runtime_emit(msg,&g_doer_b);
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
// Executes already-validated actions.
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
    char *p=line;
    while(*p==' '||*p=='\t') p++;
    if(*p=='\0') return;
    Message msg={
        .to=TARGET_A,
        .kind=MSGK_STDIN_LINE,
        .cap=1,
        .trace_id=++mint_trace_id,
        .payload=p
    };
    record_edge(msg.trace_id,0,"WORLD","stdin");
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
    printf("\n");
    for(int i=0;i<g_edge_count;i++)
    {
        if(g_edges[i].trace_id==trace_id)
        {
            printf("msg %"PRIu64" %s->%s\n",
                   g_edges[i].msg_id,
                   g_edges[i].from,
                   g_edges[i].to);
        }
    }
}
int main(void)
{
    printf("=== SCAT13: Message Topology Exposure ===\n");
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
    Message m={.to=TARGET_BOTH,.trace_id=++          mint_trace_id,.cap=1,.payload="hi Tony."};
    runtime_route(&m);
    Message m1={.to=TARGET_A,.trace_id=++mint_trace_id,.cap=1,.payload="hi Bean."};
    Message m2={.to=TARGET_B,.trace_id=++            mint_trace_id,.cap=2,.payload="hi Alex."};
    Message m3={.to=TARGET_BOTH,.trace_id=++          mint_trace_id,.cap=2,.payload="yea all"};
    runtime_route(&m1);
    runtime_route(&m2);
    runtime_route(&m3);
    Scheduler sched={.reg=&reg};
    while(g_running)
    {
        emit_stdin_event();
        while(scheduler_has_work(&sched))
        {
            scheduler_round(&sched);
        }
        runtime_print_message_balance(&reg);
    }
    
    io_uring_queue_exit(&g_ring);
    printf("io_uring is cleaned.\n");
    for(int i=0;i<mint_trace_id;i++)dump_trace(i+1); 
    return 0;
}
