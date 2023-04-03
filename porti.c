#include "env_var.h"
#include "ipc.h"

void request_gen(struct merci * tipi_merci, int * request_merci);
void offer_gen(struct merci * tipi_merci, int * offer_merci);

int * fill;

void handler_start(int signal){
}

int main(int argc, char * argv[]){

    double harbor_pos_x;
    double harbor_pos_y;
    struct sigaction sa;
    int shm_fill_id, sem_config_id, shm_merci_id;
    struct merci * tipi_merci;
    int * request_merci;
    int * offer_merci;

    srand(getpid());

    /*Creo i primi 4 porti su i 4 lati della mappa*/
    if(atoi(argv[2]) != 0){
        switch (atoi(argv[2]))
        {
            /*In alto a sinistra*/
            case 1:
                printf("In alto a sinistra\n");
                harbor_pos_x = - SO_LATO/2;
                harbor_pos_y = SO_LATO/2;
            break;
            
            /*In alto a destra*/
            case 2:
                printf("In alto a destra\n");
                harbor_pos_x = SO_LATO/2;
                harbor_pos_y = SO_LATO/2;
            break;

            /*In basso a sinistra*/
            case 3: 
                printf("In basso a sinistra\n");
                harbor_pos_x = -SO_LATO/2;
                harbor_pos_y = -SO_LATO/2;
            break;

            /*In basso a destra*/
            case 4:
                printf("In basso a destra\n");
                harbor_pos_x = SO_LATO/2;
                harbor_pos_y = -SO_LATO/2;
            break;
        }
    }
    else{
        harbor_pos_x = (rand() % (SO_LATO + 1)) - (SO_LATO / 2);
        harbor_pos_y = (rand() % (SO_LATO + 1)) - (SO_LATO /2);
    }

    /* Abbasso il semaforo per comunicare al master che il porto e' pronto */
    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);
    sem_reserve(sem_config_id, 0);

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_start;
    sigaction(SIGUSR1, &sa, NULL);

    pause();

    printf("fill: %d\n", *fill);

    shm_fill_id = shmget(getppid(), 4, 0600 | IPC_CREAT);
    fill = shmat(shm_fill_id, NULL, 0);

    shm_merci_id = shmget(getppid()+1, sizeof(tipi_merci)*SO_MERCI, 0600 | IPC_CREAT);
    tipi_merci = shmat(shm_merci_id, NULL, 0);

    /*for(i = 0; i < SO_MERCI; i++){
        printf("Tipo: %d, Peso: %d, Vita: %d\n", tipi_merci[i].type, tipi_merci[i].weight, tipi_merci[i].life);
    }*/

    while(*fill < SO_FILL){
        request_gen(tipi_merci, request_merci);
        offer_gen(tipi_merci, request_merci);
    }
}

void offer_gen(struct merci * tipi_merci, int * offer_merci){
    int merce_rand, i = 0;
    if(* fill + 5 <= SO_FILL){
        merce_rand = rand() % SO_MERCI;
        * fill += tipi_merci[merce_rand].weight;
        offer_merci[i] = tipi_merci[merce_rand].type;
        i++; 
    }
    printf("fill: %d\n", *fill);

}

void request_gen(struct merci * tipi_merci, int * request_merci){
    int merce_rand, i = 0;
    if(* fill + 5 <= SO_FILL){
        merce_rand = rand() % SO_MERCI;
        * fill += tipi_merci[merce_rand].weight;
        request_merci[i] = tipi_merci[merce_rand].type;
        i++;
    }
    printf("fill: %d\n", * fill);
}