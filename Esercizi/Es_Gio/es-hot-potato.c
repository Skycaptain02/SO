#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>


int main(int argc, char * argv[])
{
	int i, j, n_kids, number, fd_rd, fd_wr, num_bytes;
	int * pipes; /* All pipes here */

	/* Parsing command-line arguments */
	if (argc < 3) {
		fprintf(stderr, "Please provde at least <num_procs> and <init_number>\n");
		exit(1);
	}
	if ((n_kids = strtol(argv[1], NULL, 10)) <= 0) {
		fprintf(stderr, "Unable to read <num_procs> from argv[1]\n");
		exit(1);
	}
	if ((number = strtol(argv[2], NULL, 10)) <= 0) {
		fprintf(stderr, "Unable to read <init_number> from argv[2]\n");
		exit(1);
	}
	
	srand(getpid());
	/* Allocate and create the pipes */
	pipes = calloc(2*n_kids, sizeof(*pipes));
	for (i=0; i<n_kids; i++) {
		pipe(pipes+2*i);
	}
	
	/* Now forking kids */
	for (i=0; i<n_kids; i++) {
		if (fork() == 0) {
			/* Save used file descr, close others */
			fd_rd = pipes[2*i];
			j = (i == n_kids-1 ? 1 : 2*(i+1)+1);
			fd_wr = pipes[j];
			for (j = 0; j<2*n_kids; j++) {
				if (pipes[j] != fd_rd && pipes[j] != fd_wr) {
					close(pipes[j]);
				}				
			}
			free(pipes);
			
			/* Now looping read + decrement + write */
			
			while (num_bytes = read(fd_rd, &number, sizeof(number))) {
				printf("PID %d: read %d bytes, value=%d\n", getpid(), num_bytes, number);
				if (num_bytes != sizeof(number)) {
					fprintf(stderr, "Read %d instead of %ld bytes. It should not happen\n", num_bytes, sizeof(number));
					exit(1);
				}
				if (number > 0 ) {
					number--;
					write(fd_wr, &number, sizeof(number));
				} else if (number == 0) {
					write(fd_wr, &number, sizeof(number));
					break;
				} else {
					fprintf(stderr, "Read number %d. It should not happen\n", number);
					exit(1);
				}
			}
			exit(0); /* it also closes open file descriptors */
		}
	}
	
	/* Init the sequence */
	number = rand() % number + 1;
	write(pipes[1], &number, sizeof(number));
	for (j = 0; j<2*n_kids; j++) {
		close(pipes[j]);
	}
	
	while (wait(NULL) != -1);
	
}

