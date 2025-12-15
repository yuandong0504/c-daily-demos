#include <stdio.h>
#include <string.h>

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
            return C2M_OK;
        case CMD_SEND_B:
            msg->to=TARGET_B;
            msg->payload=cmd->text;
            return C2M_OK;
        case CMD_SEND_BOTH:
            msg->to=TARGET_BOTH;
            msg->payload=cmd->text;
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
    void (*handle)(Doer *self,const Message *msg);
};
static void doer_A_handle(Doer *self,const Message *msg)
{
    (void)self;
    printf("[A]:%s\n",msg->payload);
}
static void doer_B_handle(Doer *self,const Message *  msg)
{
    (void)self;
    printf("[B]:%s\n",msg->payload);
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
static void route_message(const Message *msg)
{
    switch(msg->to)
    {
        case TARGET_A:
            inbox_push(&g_doer_A.inbox,msg);
            break;
        case TARGET_B:
            inbox_push(&g_doer_B.inbox,msg);
            break;
        case TARGET_BOTH:
            inbox_push(&g_doer_A.inbox,msg);
            inbox_push(&g_doer_B.inbox,msg);
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
int main(void)
{
    char line[1024];
    runtime_init();
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
                  route_message(&msg);
                  break;
              case C2M_CTRL_EXIT:
                  return 0;
              case C2M_NOOP:
                  break;
        }
        dispatch_doer(&g_doer_A);
        dispatch_doer(&g_doer_B);
    }

    return 0;
}
