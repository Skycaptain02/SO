/* #define PROTECT_SHM */

#ifdef PROTECT_SHM
#define NUM_SEMS 2
#define LOCK					\
	sops.sem_num = 1;			\
	sops.sem_op = -1;			\
	sops.sem_flg = 0;			\
	semop(s_id, &sops, 1);
#define UNLOCK					\
	sops.sem_num = 1;			\
	sops.sem_op = 1;			\
	sops.sem_flg = 0;			\
	semop(s_id, &sops, 1);
#else
#define NUM_SEMS 1
#define LOCK
#define UNLOCK
#endif  /* PROTECT_SHM */


#define NUM_PROC  4
#define NUM_ITER  30
#define TEST_ERROR    if (errno) {dprintf(STDERR_FILENO,		\
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}


/*
 * Declaration of the type of the shared data structure
 */
struct shared_data {
	/* index where next write will happen */
	unsigned long cur_idx;
	/*
	 * vector that contains the PIDS of the process which accessed
	 * the shared memory
	 */
	pid_t vec[NUM_PROC*NUM_ITER];  
	int j[NUM_PROC*NUM_ITER];  
};

/*
 * Print status of the shared memory m_id onto file descriptor fd
 */
static void shm_print_stats(int fd, int m_id);
