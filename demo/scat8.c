#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

static void die(const char *msg)
{
    perror(msg);
    _exit(1);
}
static ssize_t read_line(char **out)
{
    printf(">>>");
    fflush(stdout);
    ssize_t n=0;
    char *line=NULL;
    size_t cap=0;
    for(;;)
    {
        n=getline(&line,&cap,stdin);
        if(n==-1)
        {
            if(errno==EINTR){continue;}
            perror("getline");
            free(line);
            *out=NULL;
            return -1;
        }
        if(line[n-1]=='\n'){line[n-1]='\0';}
        break;
    }
    *out=line;
    return n;
}
static char **parse(char *line,int *argc_out)
{
    char **argv=NULL;
    int argc=0;
    size_t cap=0;
    char *tok=strtok(line," ");
    while(tok)
    {
        if(argc+1>=cap)
        {
            size_t new_cap=cap?cap*2:8;
            char **tmp=realloc(argv,new_cap*sizeof(*argv));
            if(!tmp){die("realloc");}
            argv=tmp;
            cap=new_cap;
        }
        argv[argc++]=tok;
        tok=strtok(NULL," ");
    }
    if(argc+1>=cap)
    {
        size_t new_cap=cap?cap*2:8;
        char **tmp=realloc(argv,new_cap*sizeof(*argv));
        if(!tmp){die("realloc");}
        argv=tmp;
        cap=new_cap;
    }
    argv[argc]=NULL;

    if(argc_out){*argc_out=argc;}
    return argv;
}
void run_cmd(char **argv)
{
    pid_t pid=fork();
    if(pid==-1){die("fork");}
    if(pid==0)
    {
        execvp(argv[0],argv);
        perror("execvp");
        exit(1);
    }
    int status=0;
    waitpid(pid,&status,0);
}
int main(void)
{
    printf("pid=%d\n",getpid());
    for(;;)
    {
        char *line=NULL;
        ssize_t n=read_line(&line);
        if(n==-1){return 1;}
        printf("line=%s\n",line);
        int argc=0;
        char **argv=parse(line,&argc);
        run_cmd(argv);

        free(argv);
        free(line);
    }
    return 0;
}

