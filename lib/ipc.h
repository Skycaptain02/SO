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

int sem_remove(int sem_id);

/*Fine Sez Semafori*/

/*Sez Message Queue*/

struct msqid_ds{
	int a;
};

int msg_send(int msg_id, const void *msg_p, size_t msg_size, int msg_flags);

int msg_receive(int msg_id, const void *msg_p, size_t msg_size, long msg_type, int msg_flags);

int msg_control(int msg_id, int msg_command, struct msqid_ds *msg_buf);

/*Fine Sez Message Queue*/



#endif