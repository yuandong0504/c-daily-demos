#define _GNU_SOURCE
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

enum handler_result{
    HR_OK=0,
    HR_DEL=-1
};
struct handler{
    int fd;
    enum handler_result (*fn)(int fd);
};
static void die(const char *msg)
{
    perror(msg);
    _exit(1);
}
static void set_nonblock(int fd)
{
    int flags=fcntl(fd,F_GETFL,0);
    if(flags==-1)die("fcntl GETFL");
    if(fcntl(fd,F_SETFL,flags|O_NONBLOCK)==-1)
        die("fcntl SETFL");
}
enum handler_result handle_timer(int tfd)
{
    unsigned long long exp;
    for(;;)
    {
        ssize_t n=read(tfd,&exp,sizeof(exp));
        if(n>0)break;
        if(n==-1&&errno==EINTR)continue;
        return HR_DEL;
    }
    printf("expirations:%llu\n",exp);
    return HR_OK;
}
enum handler_result handle_stdin(int fd)
{
    char buf[1024];
    ssize_t n=read(fd,buf,sizeof(buf)-1);
    if(n=-1)
    {
        if(errno==EINTR)return HR_OK;
        return HR_DEL;
    }
    if(n==0)
    {
        printf("EOF,unregistered.\n");
        return HR_DEL;
    }
    buf[n]='\0';
    printf("received:%s\n",buf);
    return HR_OK;
}
enum handler_result handle_pipe(int pfd)
{
    char buf[1024];
    ssize_t n=read(pfd,buf,sizeof(buf)-1);
    if(n=-1)
    {
        if(errno==EINTR)return HR_OK;
        return HR_DEL;
    }
    if(n==0)
    {
        printf("EOF,unregistered.\n");
        return HR_DEL;
    }
    buf[n]='\0';
    printf("received:%s\n",buf);
    return HR_OK;
}
static void event_loop(int epfd,struct handler *hs,int count)
{
    struct epoll_event events[64];
    for(;;)
    {
        int n=epoll_wait(epfd,events,64,-1);
        if(n==-1)die("epoll_wait");
        for(int i=0;i<n;i++)
        {
            int fd= events[i].data.fd;
            for(int k=0;k<count;k++)
            {
                if(fd==hs[k].fd)
                {
                    enum handler_result r=hs[k].fn(fd);
                    if(r==HR_DEL)
                        epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
                    break;
                }
            }
        }
    }
}
int main(void)
{
    printf("pid=%d\n",getpid());
    int epfd=-1;
    int tfd=-1;
    int pfd[2]={-1,-1};
    /*init epfd*/
    epfd=epoll_create1(0);
    if(epfd==-1)die("epoll_create");
    struct epoll_event ev={.events=EPOLLIN};
    /*init tfd*/
    tfd=timerfd_create(CLOCK_MONOTONIC,0);
    if(tfd==-1)die("timerfd_create");
    set_nonblock(tfd);
    struct itimerspec its={{3,0},{3,0}};
    if(timerfd_settime(tfd,0,&its,NULL)==-1)die("timerfd_settime");
    /*init pfd*/
    if(pipe(pfd)==-1)die("pipe(pfd)");
    set_nonblock(pfd[0]);
    /*register tfd,pfd[0],stdin*/
    ev.data.fd=tfd;
    if(epoll_ctl(epfd,EPOLL_CTL_ADD,tfd,&ev)==-1)die("epoll_ctl add");
    ev.data.fd=pfd[0];
    if(epoll_ctl(epfd,EPOLL_CTL_ADD,pfd[0],&ev)==-    1)die("epoll_ctl add");
    set_nonblock(STDIN_FILENO);
    ev.data.fd=STDIN_FILENO;
    if(epoll_ctl(epfd,EPOLL_CTL_ADD,STDIN_FILENO,&ev)==-1)die("epoll_ctl add");
    /*fork child,pfd[1] provide event*/
    if(fork()==0)
    {
        for(;;)
        {
            sleep(5);
            ssize_t n=write(pfd[1],"hello",5);
            if(n==-1)
            {
                if(errno==EINTR)continue;
                if(errno==EAGAIN)continue;
                perror("child pipe write");
                _exit(1);
            }
        }
        _exit(0);
    }
    /*event_loop*/
    struct handler hs[3]={
        {tfd,handle_timer},
        {pfd[0],handle_pipe},
        {STDIN_FILENO,handle_stdin}
    };
    event_loop(epfd,hs,3);
    return 0;
}
