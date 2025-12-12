#include <stdio.h>
#include <string.h>

typedef enum{
    CMD_SEND_A,
    CMD_SEND_B,
    CMD_SEND_BOTH,
    CMD_EXIT,
    CMD_UNKNOWN
}commandType;
typedef struct command{
    commandType type;
    char *text;
}command;
static command parse_command(char *line)
{
    command cmd={.type=CMD_UNKNOWN,.text=NULL};
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
        command cmd=parse_command(line);
        switch(cmd.type)
        {
              case CMD_SEND_A:
                  printf("from a:%s\n",cmd.text);
                  break;
              case CMD_SEND_B:
                  printf("from b:%s\n",cmd.text);
                  break;
              case CMD_SEND_BOTH:
                  printf("from a:%s\n",cmd.text);
                  printf("from b:%s\n",cmd.text);
                  break;
              case CMD_EXIT:
                  printf("exit entered\n");
                  break;
              default:
                  printf("unknown commandd\n");
                  break;
        }
    }

    return 0;
}
