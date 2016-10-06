#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

extern const char * const sys_siglist[];
void my_handler(int sig,siginfo_t *siginfo,void *context) {
	printf("%s from %ld",sys_siglist[sig],(long)siginfo->si_pid);
	exit(0);
}

int main(int argc,char* argv[]) {
	struct sigaction sigact;
	sigact.sa_sigaction = &my_handler;
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
