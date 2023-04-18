#include "env_var.h"
#include "../lib/ipc.h"

void request_gen(struct merci * tipi_merci, int * request_merci);
void offer_gen(struct merci * tipi_merci, int * offer_merci);

int * fill;

void handler_start(int signal){
}

int main(int argc, char * argv[]){

    double harbor_pos_x;
    double harbor_pos_y;

    struct sigaction sa;
    int shm_fill_id, shm_merci_id, shm_pos_id, shm_richieste_id, shm_offerte_id;
    int sem_config_id, sem_offerte_richieste_id;
    struct merci * tipi_merci;
    int * richieste, * offerte;
    double * arr_pos;
    int i;

    richieste = malloc(sizeof(int) * SO_PORTI * (SO_MERCI + 1));
    offerte = malloc(sizeof(int) * SO_PORTI * (SO_MERCI + 1));

    srand(getpid());

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_start;
    sigaction(SIGUSR1, &sa, NULL);

/**
 * Creo i primi 4 porti su i 4 lati della mappa
 * La mappa e' stata configurata in modo che il centro di essa sia nelle coordinate x = 0; y = 0;
 * Come richiesto da consegna, i primi 4 porti sono collocati almeno in un lato della mappa, come posizione abbiamo scelte gli angoli del quadrato
*/

    if(atoi(argv[1]) != 0){
        switch (atoi(argv[1]))
        {
            /*In alto a sinistra*/
            case 1:
                harbor_pos_x = - SO_LATO/2;
                harbor_pos_y = SO_LATO/2;
            break;
            
            /*In alto a destra*/
            case 2:
                harbor_pos_x = SO_LATO/2;
                harbor_pos_y = SO_LATO/2;
            break;

            /*In basso a sinistra*/
            case 3: 
                harbor_pos_x = -SO_LATO/2;
                harbor_pos_y = -SO_LATO/2;
            break;

            /*In basso a destra*/
            case 4:
                harbor_pos_x = SO_LATO/2;
                harbor_pos_y = -SO_LATO/2;
            break;
        }
    }
    else{
        harbor_pos_x = (rand() % (SO_LATO + 1)) - (SO_LATO / 2);
        harbor_pos_y = (rand() % (SO_LATO + 1)) - (SO_LATO /2);
    }


    /**
     * Porti creati in una posizione casuale della mappa, comunico al master tramite semaforo che tutti i porti sono pronti a comiciare la simulazione
    */
    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);
    sem_reserve(sem_config_id, 0);


    shm_fill_id = shmget(getppid(), 4, 0600 | IPC_CREAT);
    fill = shmat(shm_fill_id, NULL, 0);

    shm_merci_id = shmget(getppid() + 1, sizeof(tipi_merci) * SO_MERCI, 0600 | IPC_CREAT);
    tipi_merci = shmat(shm_merci_id, NULL, 0);

    sem_offerte_richieste_id = semget(getppid() + 1, 1, 0600 | IPC_CREAT);
    while(semctl(sem_offerte_richieste_id, 0, GETVAL) != 0);

    pause();


    /**
     * Creazione e allocazione in shared memory dell'array il cui contenuto corrisponderà alla posizione 
     * di ti tutti i porti all'interno della mappa
    */

    shm_pos_id = shmget(getppid() + 5, sizeof(double) * (SO_PORTI * 3), 0600 | IPC_CREAT);
    arr_pos = shmat(shm_pos_id, NULL, 0);

    for(i = 0; i < SO_PORTI; i++){
        if(arr_pos[i * SO_PORTI] == getpid()){
            arr_pos[i * SO_PORTI + 1] = harbor_pos_x;
            arr_pos[i * SO_PORTI + 2] = harbor_pos_y;
        }
    }


}


void offer_gen(struct merci * tipi_merci, int * offer_merci){

}

void request_gen(struct merci * tipi_merci, int * request_merci){
}
