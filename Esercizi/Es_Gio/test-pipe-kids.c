#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#define NUM_PROC      8
#define NUM_ITER      100
#define READ_BUF_SIZE 5        /* try varying this */
#define TEST_ERROR    if (errno) {fprintf(stderr, \
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
	
	
	/* 
	 * Create a pipe: any byte written to the my_pipe[1] file
	 * descriptor can be read from the mypipe[0] file descriptor
	 */
	pipe(my_pipe);
	TEST_ERROR;
	for (i=0; i<NUM_PROC; i++) {
		switch (fork()) {
		case -1:
			/* Handle error */
			TEST_ERROR;
			exit(EXIT_FAILURE);
		case 0:
			/* 
			 * Child process will only write, the read end
			 * must be closed
			 */
			close(my_pipe[0]);
			TEST_ERROR;
			
			/* Printing NUM_ITER rows to the pipe */
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
			break;
		}
	}
	
	/* 
	 * Parent process will only read, the write end must be
	 * closed. If the write end is not closed, the read will never
	 * return EOF!!!
	 */
	close(my_pipe[1]);
	TEST_ERROR;
	
	/* Allocated read buffer */
	buf = malloc(READ_BUF_SIZE);
	TEST_ERROR;

	/*
	 * The number of bytes read is not necessarily equal to the
	 * length of the rows written by child processes. If only one
	 * reader, it doesn't matter. If two readers then we must be
	 * careful...
	 */
#if 0
	fork();
#endif
	while (num_bytes = read(my_pipe[0], buf, READ_BUF_SIZE)) {
		write(STDOUT_FILENO, buf, num_bytes);
		TEST_ERROR;
	}
	/* De-allocate read buffer */
	free(buf);
	/* done with reading */
	close(my_pipe[0]);
	
	/* Now let's wait for the termination of all kids */
	while ((child_pid = wait(&status)) != -1) {
		printf("PARENT: PID=%d. Got info of child with PID=%d, status=0x%04X\n", getpid(), child_pid,status);
	}
	
	exit(EXIT_SUCCESS);
}
