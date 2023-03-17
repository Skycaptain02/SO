#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <string.h>


int main (void)
{
	pid_t child_pid;
	int status;
	
	/* Preparing environment variables for invoking execve */
	char* env_vars[] = {
		"VAR1=variabile1",
		"HOME=boh",
		"questa_senza_uguale",
		NULL
	};
	
	/* Preparing command-line arguments for invoking execve */
	char* args[] = {
		"name_command",   /* */
		"VAR1",
		"AAAAAAAAAAAAAAAA",
		"BBBBBBBBBBBBBBBB",		
		NULL
	};

	printf("%s: My PID is %d and my parent's PID is %d\n",
	       __FILE__, getpid(), getppid());

	execve("testsrrjfuyrcbwrg-getpid", args, env_vars);
	/* execve("./test-env", args, env_vars); */
	/* execve("./test-command-line", args, env_vars);*/
 	/* execve("./test-getpid", args, env_vars);*/
	
	/* don't need to check success. If here not success */
	fprintf(stderr, "%s: %d. Error #%03d: %s\n",
		__FILE__, __LINE__, errno, strerror(errno));
	
	exit(EXIT_FAILURE);
}
