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

#define CHILD_NAME "test-shm-child"

int main() {
	int m_id, s_id;
	char m_id_str[3*sizeof(m_id)+1]; /* sufficiente per stringa m_id */
	char s_id_str[3*sizeof(s_id)+1]; /* sufficiente per stringa s_id */
	int cur_i, i, j, status;
	struct shared_data * my_data;
	pid_t child_pid, my_pid, *kid_pids;
	struct sembuf sops;
	unsigned long * count_writes;
	char * args[4] = {CHILD_NAME};  /* deve essere nome eseguibile */



	
	/* Create a shared memory area */
	m_id = shmget(IPC_PRIVATE, sizeof(*my_data), 0600);
	TEST_ERROR;
	/* Attach the shared memory to a pointer */
	my_data = shmat(m_id, NULL, 0);
	TEST_ERROR;
	/* 
	 * Marking immediately  the shm for deallocation.  Just not to
	 * forget.   Remember:  it  will  be  deleted  only  when  all
	 * processes are detached from it!!
	 */
	shmctl(m_id, IPC_RMID, NULL);
	
	my_data->cur_idx = 0;   /* init first counter */
	shm_print_stats(2, m_id);
	
	/* Create a semaphore to synchronize the start of all child
	 * processes */
	s_id = semget(IPC_PRIVATE, NUM_SEMS, 0600);
	TEST_ERROR;
	/* Sem 0 to syncronize the start of child processes */
	semctl(s_id, 0, SETVAL, 0);
	TEST_ERROR;
#ifdef PROTECT_SHM
	semctl(s_id, 1, SETVAL, 1);
#endif	
	
	/* Preparing command-line arguments for child's execve */
	sprintf(m_id_str, "%d", m_id);
	sprintf(s_id_str, "%d", s_id);
	args[1] = m_id_str;    /* stringa con m_id */
	args[2] = s_id_str;    /* stringa con s_id */
	args[3] = NULL;        /* NULL-terminated */

	/* Initialize the common fields */
	sops.sem_num = 0;     /* check the 0-th semaphore */
	sops.sem_flg = 0;     /* no flag */
	
	/* Loop to create NUM_PROC child processes */
	kid_pids = malloc(NUM_PROC*sizeof(*kid_pids));
	count_writes = calloc(NUM_PROC, sizeof(*count_writes));
	for (i=0; i<NUM_PROC; i++) {
		switch (kid_pids[i] = fork()) {
		case -1:
			TEST_ERROR;
			break;
		case 0:
			execve(CHILD_NAME, args, NULL); /* no env variables */
			TEST_ERROR;
			break;
		default:
			break;
		}
	}
	
	/* Inform child processes to start writing to the shared mem */
	sops.sem_op = NUM_PROC;
	semop(s_id, &sops, 1);
	
	/* Waiting for all child processes to terminate */
	while ((child_pid = wait(&status)) != -1) {
		dprintf(2,"PID=%d. Sender (PID=%d) terminated with status 0x%04X\n",
			getpid(),
			child_pid,
			status);
	}
	
	/* All kids are terminated: the semaphore may be deallocated */
	semctl(s_id, 0, IPC_RMID);
	
	/* Now print the content of the shared data structure */
	shm_print_stats(2, m_id);
	for (i=0; i<NUM_PROC*NUM_ITER; i++) {
	  printf("vec[%6d]: pid=%d, j=%d\n", i, my_data->vec[i], my_data->j[i]);
		for (j=0; j<NUM_PROC; j++) {
			if (my_data->vec[i] == kid_pids[j]) {
				count_writes[j]++;
			}
		}
	}
	for (j=0; j<NUM_PROC; j++) {
		printf("PID=%d: NUM_ITER=%d, count_writes[j]=%ld\n",
		       kid_pids[j], NUM_ITER, count_writes[j]);
	}
	exit(0);
}


static void shm_print_stats(int fd, int m_id) {
	struct shmid_ds my_m_data;
	int ret_val;
	
	while (ret_val = shmctl(m_id, IPC_STAT, &my_m_data)) {
		TEST_ERROR;
	}
	dprintf(fd, "--- IPC Shared Memory ID: %8d, START ---\n", m_id);
	dprintf(fd, "---------------------- Memory size: %ld\n",
		my_m_data.shm_segsz);
	dprintf(fd, "---------------------- Time of last attach: %ld\n",
		my_m_data.shm_atime);
	dprintf(fd, "---------------------- Time of last detach: %ld\n",
		my_m_data.shm_dtime); 
	dprintf(fd, "---------------------- Time of last change: %ld\n",
		my_m_data.shm_ctime); 
	dprintf(fd, "---------- Number of attached processes: %ld\n",
		my_m_data.shm_nattch);
	dprintf(fd, "----------------------- PID of creator: %d\n",
		my_m_data.shm_cpid);
	dprintf(fd, "----------------------- PID of last shmat/shmdt: %d\n",
		my_m_data.shm_lpid);
	dprintf(fd, "--- IPC Shared Memory ID: %8d, END -----\n", m_id);
}
