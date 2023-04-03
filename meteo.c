#include "env_var.h"
#include "ipc.h"

int main(int argc, char * argv[]){
    int sem_config_id;

    /*Sezione creazione semaforo per configurazione*/
    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);
    sem_reserve(sem_config_id, 0);
    /*Fine Sezione*/
}