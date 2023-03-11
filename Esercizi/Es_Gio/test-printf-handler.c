/*
 * Try invoking
 *
 * ./a.out 10000
 *
 * and
 *
 * ./a.out 10000 > output
 *
 * File test-printf-handler.output is an example of output. Interesting
 * behaviour at rows: 116 (easy to explain), 323 (not so easy), 1276, 1887,
 */

#define _GNU_SOURCE    /* NECESSARY from now on */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <strings.h>


unsigned long count_in_handler = 0;

/* SIGUSR1 handler: used by child process */
void handle_signal(int signal) {
	
	printf("In handler:\t%ld\n", count_in_handler++);
}

/* SIGCHLD handler: used by parent process */
void handle_child(int signal) {
	
	while (wait(NULL) != -1 || errno != ECHILD);
	exit(0);
}


int main(int argc, char * argv[])
{
	int child_pid;
	unsigned long i, n_iter;
	struct sigaction sa;
	
	/* Reading number of iterations from command-line */
	if (argc < 2) {
		fprintf(stderr, "Need a number as command-line argument\n");
		return 1;
	}
	if ((n_iter = strtol(argv[1], NULL, 10)) <= 0) {
		fprintf(stderr, "Need a number as command-line argument\n");
		return 1;
	}
	
	/* Setting handler */
	bzero(&sa, sizeof(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGUSR1, &sa, NULL);
	sa.sa_handler = handle_child;
	sigaction(SIGCHLD, &sa, NULL);
	
	/* QUESTION: can I set the handlers after the fork? */
	if (child_pid = fork()) {
		/* Parent sending SIGUSR1 forever, until child terminates */
		for(;;) {
			kill(child_pid, SIGUSR1);
			usleep(1);
		}
	} else {
		/* Child printing lines here and in handler */
		for(i=0; i<n_iter; i++) {
			printf("In main:\t%ld\n", i);
		}
		/* Now finally, ignoring SIGUSR1 */
		sa.sa_handler = SIG_IGN;
		sigaction(SIGUSR1, &sa, NULL);
		printf("In main (total):\t%ld\n", i);
		printf("In handler (total):\t%ld\n", count_in_handler);
		return 0;
	}
}
