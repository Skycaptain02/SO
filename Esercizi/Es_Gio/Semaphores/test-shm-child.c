#define _GNU_SOURCE  /* Per poter compilare con -std=c89 -pedantic */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "test-shm.h"

int main(int argc, char * argv[]) {
	int m_id, s_id;
	int cur_i, j;
	struct shared_data * my_data;
	pid_t my_pid;
  	struct sembuf sops;
	
	/* Getting the IDs of IPC object from command line */
	m_id = atoi(argv[1]);
	s_id = atoi(argv[2]);

	/* Attach the shared memory to a pointer */
	my_data = shmat(m_id, NULL, 0);
	TEST_ERROR;

	my_pid = getpid();
	
	/* Wait for the "green light" by the parent */
	sops.sem_op = -1;
	semop(s_id, &sops, 1);
	
	/* Child process writes NUM_ITER times */
	for(j=0; j<NUM_ITER; j++) {
		/*
		 *  Child process writes at current
		 *  position, then increment
		 */
		LOCK;
		cur_i = my_data->cur_idx;
		my_data->cur_idx++;
		UNLOCK;
		my_data->vec[cur_i] = my_pid;
		my_data->j[cur_i] = j;
	}
	/*
	 * Done writing,  now child process  can exit.
	 * The  exit system  call also  detaches child
	 * processes from the shared memory
	 */
	exit(0);
}
