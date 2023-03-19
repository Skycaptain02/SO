#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#define NUM_KIDS 8

int main (int argc, char * argv[]) {
	long num_cores, n_loops;
	pid_t child_pid;
	int status, i, j;
	
	/* Looping to create child processes */ 
	for (i=0; i<NUM_KIDS; i++) {
		switch (child_pid = fork()){
		case -1:
			/* Handle error */
			fprintf(stderr, "Error with the fork\n");
			exit(EXIT_FAILURE);
		case 0:
			srand(getpid());
			int r_num = (rand() % 6) + 1;
			printf("pid = %d, r_num = %d\n", getpid(), r_num);
			exit(r_num);
			break;

		default:
			/* Perform actions specific to parent */
			printf("PID= %5d (PARENT): parent of a child with PID %d\n", getpid(), child_pid);
			break;
		}
	}
	int sum = 0;
	while ((child_pid = wait(&status)) != -1) {
		printf("PID= %5d (PARENT): Got info of child with PID=%d, status=0x%04X\n",getpid(), child_pid, status);
		status = WEXITSTATUS(status); //
		sum = sum + status;
	}
	if (errno == ECHILD) {
		printf("In PID=%6d, no more child processes\n",getpid());
		printf("%d\n", sum);
		exit(EXIT_SUCCESS);
	} else {
		fprintf(stderr, "Error #%d: %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}
