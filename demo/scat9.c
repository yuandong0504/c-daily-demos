/*
 * Runtime v0.1
 *
 * Design invariants:
 * 1. All Message creation happens in runtime_emit
 * 2. Every Message is either:
 *    - handled
 *    - pending
 *    - dropped
 * 3. Message balance must close to zero
 *
 * This file intentionally avoids:
 * - threads
 * - locks
 * - time slicing
 *
 * Tick is not fundamental.Step is.
 */

#include <stdio.h>
#include <string.h>

static unsigned long g_msg_created  = 0;
static unsigned long g_msg_enqueued = 0;
static unsigned long g_msg_handled  = 0;
static unsigned long g_msg_dropped  = 0;

static int g_msg_id=0;
static int g_cap_id=0;

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
typedef struct{
    int id;
    int cap;
    Target to;
    char *payload;
}Message;
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
    g_msg_enqueued++;
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
static void doer_A_handle(Doer *self,const Message *msg)
{
    (void)self;
    printf("msg %d cap %d [A]:%s\n",msg->id,msg->cap,msg->payload);
}
static void doer_B_handle(Doer *self,const Message *  msg)
{
    (void)self;
    printf("msg %d cap %d [B]:%s\n",msg->id,msg->cap,msg->payload);
}
static Doer g_doer_A;
static Doer g_doer_B;
static void runtime_init(void)
{
    g_doer_A.name="A";
    g_doer_A.handle=doer_A_handle;
    inbox_init(&g_doer_A.inbox);

    g_doer_B.name="B";
    g_doer_B.handle=doer_B_handle;
    inbox_init(&g_doer_B.inbox);
}
static int runtime_new_cap(void)
{
    return ++g_cap_id;
}
static void runtime_record_drop(const Message *m, Doer *d)
{
    g_msg_dropped++;
    printf("[DROP] msg=%d to=%s payload='%s'\n",
           m->id, d->name, m->payload);
}
static void runtime_emit(const Message *src,Doer *d)
{
    Message m=*src;
    g_msg_created++;
    m.id=++g_msg_id;
    m.cap=runtime_new_cap();
    if (inbox_push(&d->inbox, &m) != 0)
    {
        runtime_record_drop(&m, d);
    }
}
static void runtime_route(const Message *msg)
{
    switch(msg->to)
    {
        case TARGET_A:
            runtime_emit(msg,&g_doer_A);
            break;
        case TARGET_B:
            runtime_emit(msg,&g_doer_B);
            break;
        case TARGET_BOTH:
            runtime_emit(msg,&g_doer_A);
            runtime_emit(msg,&g_doer_B);
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
    printf("[MSG_BALANCE] created=%lu enqueued=%lu handled=%lu dropped=%lu pending=%lu balance=%ld\n",
       g_msg_created, g_msg_enqueued, g_msg_handled, g_msg_dropped,pending, balance);
}
static void scheduler_round(Scheduler *s)
{
    for(int i=0;i<s->reg->count;i++)
    {
        Doer *d=s->reg->list[i];
        Message m;
        if(inbox_pop(&d->inbox,&m)==0)
        {
            d->handle(d,&m);
            g_msg_handled++;
        }
    }
}
int main(void)
{
    // TODO(runtime):
    // inbox_push / inbox_pop / route_message
    // must be owned by runtime layer
    runtime_init();
    DoerRegistry reg;
    registry_init(&reg);
    registry_add(&reg,&g_doer_A);
    registry_add(&reg,&g_doer_B);
    Message m={.to=TARGET_BOTH,.payload="hi Tony."};
    runtime_route(&m);
    Message m1={.to=TARGET_A,.payload="hi 大哥."};
    Message m2={.to=TARGET_B,.payload="hi 小弟."};
    Message m3={.to=TARGET_BOTH,.payload="both"};
    runtime_route(&m1);
    runtime_route(&m2);
    runtime_route(&m3);
    Scheduler sched={.reg=&reg};
    while(scheduler_has_work(&sched))
    {
        scheduler_round(&sched);
    }
    runtime_print_message_balance(&reg);
    /**char line[1024];
    while(printf(">>>"),fflush(stdout),fgets(line,sizeof(line),stdin))
    {
        size_t len=strlen(line);
        if(len>0&&line[len-1]=='\n')
        {
            line[len-1]='\0';
        }
        Command cmd=parse_command(line);
        Message msg={0};
        C2MResult r=command_to_message(&cmd,&msg);
        switch(r)
        {
              case C2M_OK:
                  runtime_route(&msg);
                  break;
              case C2M_CTRL_EXIT:
                  return 0;
              case C2M_NOOP:
                  break;
        }
        dispatch_doer(&g_doer_A);
        dispatch_doer(&g_doer_B);
    }**/

    return 0;
}
