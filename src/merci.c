#include "env_var.h"

int main(int argc, char * argv[]){
    int i, shm_id;
    Merce * tipi_merci;
    srand(getpid());

    shm_id = shmget(getppid() + 1, sizeof(Merce) * SO_MERCI, 0600 | IPC_CREAT);
    tipi_merci = shmat(shm_id, NULL, 0);
    
    for(i = 0; i < SO_MERCI; i++){
        tipi_merci[i].type = (i+1);
        tipi_merci[i].weight = ((rand() % SO_SIZE) + 1);
        tipi_merci[i].life = ((rand() % SO_MAX_VITA) + SO_MIN_VITA);
    }

    if(PRINT_MERCI){
        for(i = 0; i < SO_MERCI; i++){
            printf("Tipo -> %d, Peso -> %d, Vita -> %d\n", tipi_merci[i].type, tipi_merci[i].weight, tipi_merci[i].life);
        }
    }


}