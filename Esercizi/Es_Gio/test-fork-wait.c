#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#define NUM_KIDS 20

int main (int argc, char * argv[]) {
	long num_cores, n_loops;
	pid_t child_pid;
	int status, i, j;
	
	/* Looping to create child processes */ 
	for (i=0; i<NUM_KIDS; i++) {
		switch (child_pid = fork()) {
		case -1:
			/* Handle error */
			fprintf(stderr, "Error with the fork\n");
			exit(EXIT_FAILURE);
		case 0:
			/* Perform actions specific to child */
			srand(getpid());
			int r_num = (rand() % 6) + 1;
			printf("PID= %5d (CHILD): r_num=%d, running a lot\n", getpid(), r_num);
			exit(r_num);
			
		default:
			/* Perform actions specific to parent */
			printf("PID= %5d (PARENT): parent of a child with PID %d\n",
			       getpid(), child_pid);
			break;
		}
	}
	
	int sum = 0;
	/* Now let's wait that all kids do the stuff */
	while ((child_pid = wait(&status)) != -1) {
		printf("PID= %5d (PARENT): Got info of child with PID=%d, status=%d\n",
		       getpid(), child_pid, WEXITSTATUS(status));
		status = WEXITSTATUS(status);
		sum += status;
	}
	if (errno == ECHILD) {
		printf("In PID=%6d, no more child processes\n",
		       getpid());
		printf("Sum: %d\n", sum);
		exit(EXIT_SUCCESS);
	} else {
		fprintf(stderr, "Error #%d: %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
}
