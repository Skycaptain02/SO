#include "env_var.h"

int main(int argc, char * argv[]){
    int i, shm_id;
    struct merci * tipi_merci;
    

    srand(getpid());

    tipi_merci = malloc(SO_MERCI * sizeof(tipi_merci));

    for(i = 0; i < SO_MERCI; i++){ 
        tipi_merci[i].type = (i+1);
        tipi_merci[i].weight = ((rand() % SO_SIZE) + 1);
        tipi_merci[i].life = ((rand() % SO_MAX_VITA) + SO_MIN_VITA);
    }

    shm_id = shmget(getppid() + 1, sizeof(tipi_merci) * SO_MERCI, 0600 | IPC_CREAT);
    tipi_merci = shmat(shm_id, NULL, 0);
    printf("Shm_id_merci -> %d\n", shm_id);
}