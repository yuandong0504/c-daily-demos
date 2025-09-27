#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define WRITE_LIT(fd,LIT)\
	write(fd,LIT,sizeof(LIT)-1)
volatile sig_atomic_t stop=0;
void handle_sigalrm(int sig){
	(void)sig;
	stop=1;
	WRITE_LIT(STDOUT_FILENO,
		"\n5 seconds, timeout,bye.\n");
}
int main(){
	signal(SIGALRM,handle_sigalrm);
	alarm(5);
	printf("waiting for 5 seconds...\n");
	while(!stop){
		if(stop)break;
		WRITE_LIT (STDOUT_FILENO, "tick...\n");
		sleep(1);
	}
	printf("Exiting cleanly.\n");
} 
