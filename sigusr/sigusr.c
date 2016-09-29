#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

volatile sig_atomic_t was = 0;

void handler(int signo, siginfo_t *siginfo, void *context) {
	was = 1;
	printf("signal %d from: %d\n", signo, (int)(siginfo->si_pid));
	exit(signo);
}

int main() {
    	struct sigaction sigact;
    	sigact.sa_sigaction = *handler;
    	sigact.sa_flags |= SA_SIGINFO;
    	sigset_t block_mask;
    	sigfillset(&block_mask);
	sigact.sa_mask = block_mask;
	int i;
	for (i = 1; i < 32; ++i) {
		if (((i == SIGKILL) || (i == SIGSTOP))) continue;
		if (sigaction(i, &sigact, NULL) != 0) {
        		printf("%d error\n", i);
       		 	return errno;
    		}
	} 
   	sleep(10);
	sigprocmask(SIG_BLOCK, &sigact.sa_mask, 0);
	if (!was) {
    		printf("No signals were caught\n");
	}
    	return 0;
}
