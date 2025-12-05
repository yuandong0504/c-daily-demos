#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

ssize_t read_line(char **out)
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
            return -1;
        }
        if(line[n-1]=='\n'){line[n-1]='\0';}
        break;
    }
    *out=line;
    return n;
}
int main(void)
{
    char *line=NULL;
    ssize_t n=read_line(&line);
    if(n==-1)
    {
        free(line);
        return 1;
    }
    printf("line=%s\n",line);
    free(line);
    return 0;
}

