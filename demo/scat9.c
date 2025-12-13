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
static void dispatch_message(Message *msg)
{
    switch(msg->to)
    {
        case TARGET_A:
            printf("[A]:%s\n",msg->payload);
            break;
        case TARGET_B:
            printf("[B]:%s\n",msg->payload);
            break;
        case TARGET_BOTH:
            printf("[A]:%s\n",msg->payload);
            printf("[B]:%s\n",msg->payload);
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
        Message msg;
        C2MResult r=command_to_message(&cmd,&msg);
        switch(r)
        {
              case C2M_OK:
                  dispatch_message(&msg);
                  break;
              case C2M_CTRL_EXIT:
                  return 0;
              case C2M_NOOP:
                  break;
        }
    }

    return 0;
}
