#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define WRITE_LIT(fd,LIT)\
	write(fd,LIT,sizeof(LIT)-1)
volatile sig_atomic_t stop=0;
void handle_sigint(int sig){
	(void)sig;
	stop=1;
	WRITE_LIT(STDOUT_FILENO,
		"\n[SIGINT]:signal catched\n");
}

int main(){
	signal(SIGINT,handle_sigint);
	printf("Running...press Ctrl+C to stop.\n");
	while(!stop){
		sleep(1);
		if(stop)break;
		WRITE_LIT(STDOUT_FILENO,"tick...\n");
	}
	printf("Exiting cleanly.\n");
}
