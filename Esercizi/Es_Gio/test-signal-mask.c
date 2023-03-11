#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h> /* sigaction(), sig*() */

/*
 * Handler of a signal is a function taking an integer as input and
 * producing no output
 */
void handle_signal(int signal);

int main()
{
	int time_to_wait;
	struct sigaction sa;
	sigset_t my_mask;

	/* Print pid, so that we can send signals from other shells */
	printf("My pid is: %d\n", getpid());


	/* Blocking SIGINT for the process: if generated, it will be pending */
	sigemptyset(&my_mask);                  /* set an empty mask */
	sigaddset(&my_mask, SIGINT);            /* add SIGINT to the mask */
	sigprocmask(SIG_BLOCK, &my_mask, NULL); /* mask SIGINT in the process */

	/* Setting the handler for SIGINT */
	bzero(&sa, sizeof(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGINT, &sa, NULL);
	
	/* Waiting 10 seconds until SIGINT is unblocked */
	for(time_to_wait = 10; time_to_wait; time_to_wait--) {
		printf("Waiting %d seconds until SIGINT is unblocked\n",
		       time_to_wait);
		sleep(1);
	}

	/* Unblocking now SIGINT */
	sigaddset(&my_mask, SIGINT);            /* add SIGINT to the mask */
	sigprocmask(SIG_UNBLOCK, &my_mask, NULL); /* ublock SIGINT */
	printf("Now SIGINT is unblocked\n");
	
	for (;;) {
		printf("Sleeping for ~3 seconds\n");
		sleep(3);
	}
}

void handle_signal(int signal) {
	static int count_invocations = 0;
	
	count_invocations++;
	printf("Handling signal #%d (%s): invocation %d\n",
	       signal, strsignal(signal), count_invocations);
}
