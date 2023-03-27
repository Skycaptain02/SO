#include "ipc.h"
#include "env_var.h"

/*Sez Semafori*/

int errno;

int sem_set_val(int sem_id, int sem_num, int sem_val){
    int ret_value;
    ret_value = semctl(sem_id, sem_num, SETVAL , sem_val);
    if(!errno){
        return ret_value;
    }
    else{
        printf("Error in setting semaphore value, error: %d\n", strerror(errno));
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

int sem_remove(int sem_id){
    int ret_value;
    ret_value = semop(sem_id, NULL, IPC_RMID);
    if(!errno){
        return ret_value;
    }
    else{
        printf("Error in setting semaphore value, error: %d\n", strerror(errno));
    }
}

/*Fine Sez Semafori*/

/*Sez Message Queue*/

int msg_send(int msg_id, const void *msg_p, size_t msg_size, int msg_flags){
    int ret_value;
    ret_value = msgsnd(msg_id, &msg_p, msg_size, msg_flags);
    if(!errno){
        return ret_value;
    }
    else{
        printf("Error in setting semaphore value, error: %d\n", strerror(errno));
    }
}

int msg_receive(int msg_id, const void *msg_p, size_t msg_size, long msg_type, int msg_flags){
    int ret_value;
    ret_value = msgsnd(msg_id, &msg_p, msg_size, msg_type, msg_flags);
    if(!errno){
        return ret_value;
    }
    else{
        printf("Error in setting semaphore value, error: %d\n", strerror(errno));
    }
}


int msg_control(int msg_id, int msg_command, struct msqid_ds *msg_buf){
    int ret_value;
    ret_value =  msgctl(msg_id, msg_command, msg_buf);
    if(!errno){
        return ret_value;
    }
    else{
        printf("Error in setting semaphore value, error: %d\n", strerror(errno));
    }
}

/*Fine Sez Message Queue*/
