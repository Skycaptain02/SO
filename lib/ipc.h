#ifdef IPC
#else
#define IPC
#include <stddef.h>

/*Sez Semafori*/

union semun {
	int              val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
	struct seminfo  *__buf;  /* Buffer for IPC_INFO
				    (Linux-specific) */
};

int sem_set_val(int sem_id, int sem_num, int sem_val);

int sem_reserve(int sem_id, int sem_num);

int sem_release(int sem_id, int sem_num);

/*Fine Sez Semafori*/



#endif