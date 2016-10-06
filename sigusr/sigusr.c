#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

void handler(int signal,siginfo_t *signalinfo,void *context) {
        if( signal == SIGUSR1 )
		printf("SIGUSR1 from %ld",(long)signalinfo->si_pid);
	else
		printf("SIGUSR2 from %ld",(long)signalinfo->si_pid);
	exit(0);
}


int main(int argc,char* argv[]) {

	struct sigaction sigact;
	sigact.sa_sigaction = &handler;
	sigset_t block_mask;
	sigemptyset(&block_mask);
	sigaddset(&block_mask,SIGUSR1);
	sigaddset(&block_mask,SIGUSR2);
	sigact.sa_mask = block_mask;
	sigact.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1,&sigact,NULL);
	sigaction(SIGUSR2,&sigact,NULL);
        sleep(10);	
	printf("No signals were caught");
	return 0;
}
