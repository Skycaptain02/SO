#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <limits.h>

#define MY_CMD "wc -w"   /* prints the number of words read from stdin */
#define NUM_ITER 10

#if 1
#define OUT_STREAM my_stream
#else
#define OUT_STREAM my_stream
#endif

int main (void)
{
	FILE * my_stream;
	int j;
	
	/* forking a new process running MY_CMD that gets input from
	   my_stream */
	my_stream = popen(MY_CMD, "w");
	
	/* Anything that just write something to my_stream */
	for(j=0; j<NUM_ITER; j++) {
		fprintf(OUT_STREAM,"word%d\n", j);
	}
	
	/* must be closed */
	pclose(my_stream);
	
	exit(EXIT_SUCCESS);
}
