#include "env_var.h"

int main(int argc, char * argv[]){
    int i, shm_id;
    struct merci * tipi_merci;

    printf("ARGV 1: %s\n", argv[1]);

    srand(getpid());
    shm_id = atoi(argv[1]);
    tipi_merci = shmat(shm_id, NULL, 0);

    for(i = 0; i < SO_MERCI; i++){ 
        tipi_merci[i].type = (i+1);
        tipi_merci[i].weight = ((rand() % SO_SIZE)+1);
        tipi_merci[i].life = ((rand() % SO_MAX_VITA)+SO_MIN_VITA);
    }
}