#include "ipc.h"
#include "../src/env_var.h"


/*Sez Semafori*/

int errno;

int sem_set_val(int sem_id, int sem_num, int sem_val){
    int ret_value;
    ret_value = semctl(sem_id, sem_num, SETVAL , sem_val);
    if(!errno){
        return ret_value;
    }
    else{
        printf("Error in setting semaphore value, error: %s\n", strerror(errno));
    }
    
}

int sem_reserve(int sem_id, int sem_num){
    struct sembuf sem_op;
    sem_op.sem_num = sem_num;
    sem_op.sem_flg = 0;
    sem_op.sem_op = -1;
    return semop(sem_id, &sem_op, 1);
}

int sem_release(int sem_id, int sem_num){
    struct sembuf sem_op;
    sem_op.sem_num = sem_num;
    sem_op.sem_flg = 0;
    sem_op.sem_op = 1;
    return semop(sem_id, &sem_op, 1);
}
/*Fine Sez Semafori*/

