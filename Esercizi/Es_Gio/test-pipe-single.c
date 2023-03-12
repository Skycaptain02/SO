#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <limits.h>   /* to define PIPE_BUF */

/*
#define ADD_MARK_AFTER_READ
*/

#define NUM_ITER      5
#define READ_BUF_SIZE 5        /* try varying this */
#define TEST_ERROR    if (errno) {fprintf(stderr,			\
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}


int main (void)
{
	pid_t child_pid;
	int my_pipe[2];
	int i, j, status;
	char * buf;
	ssize_t num_bytes;
	
	
	/* PIPE_BUF is declared in limits.h */
	printf("Pipe capacity on this machine is %d bytes. Good.\n", PIPE_BUF);

	/* 
	 * Create a pipe: any byte written to the my_pipe[1] file
	 * descriptor can be read from the mypipe[0] file descriptor
	 */
	pipe(my_pipe);    
	TEST_ERROR;
	switch (fork()) {
	case -1:
		/* Handle error */
		TEST_ERROR;
		exit(EXIT_FAILURE);
	case 0:
		/* Perform actions specific to child */
		/* Child process will only write, the read end MUST be closed */
		close(my_pipe[0]);
		TEST_ERROR;
		
		/* Each child write something the the pipe */
		for(j=0; j<NUM_ITER; j++) {
			dprintf(my_pipe[1],
				"CHILD PID %5d: j=%2d\n", getpid(), j);
			TEST_ERROR;
		}
		
		/* Now the write end can be closed as well */
		close(my_pipe[1]);
		TEST_ERROR;
		exit(EXIT_SUCCESS);
		break;
		
	default:
		/* Perform actions specific to parent */
		/* Parent process will only read, the write end MUST
		 * be closed */
		close(my_pipe[1]);
		TEST_ERROR;

		/* Allocated read buffer */
		buf = malloc(READ_BUF_SIZE);
		TEST_ERROR;
		
		while (num_bytes = read(my_pipe[0], buf, READ_BUF_SIZE)) {
			write(1, buf, num_bytes);
			TEST_ERROR;
#ifdef ADD_MARK_AFTER_READ
			write(1, "___", 3);
#endif
		}
		free(buf);
		/* done with reading */
		close(my_pipe[0]);
		break;
	}
	
	/* Now let's wait for the termination of child process */
	while ((child_pid = wait(&status)) != -1) {
		printf("\nPARENT: PID=%d. Got info of child with PID=%d, status=0x%04X\n", getpid(), child_pid,status);
	}
	
	exit(EXIT_SUCCESS);
}
