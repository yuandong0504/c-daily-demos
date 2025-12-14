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
typedef struct Doer{
    char *name;
    void (*handle)(struct Doer *self,const Message *msg);
}Doer;
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
static Doer doer_A={
    .name="A",
    .handle=doer_A_handle
};
static Doer doer_B={
    .name="B",
    .handle=doer_B_handle
};
static void route_message(Message *msg)
{
    switch(msg->to)
    {
        case TARGET_A:
            doer_A.handle(&doer_A,msg);
            break;
        case TARGET_B:
            doer_B.handle(&doer_B,msg);
            break;
        case TARGET_BOTH:
            doer_A.handle(&doer_A,msg);
            doer_B.handle(&doer_B,msg);
            break;
    }
}
int main(void)
{
    char line[1024];
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
    }

    return 0;
}
