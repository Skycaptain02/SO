#include "env_var.h"
#include "../lib/ipc.h"

void request_gen(struct merci * tipi_merci, int * request_merci, int perc_richieste, int matr_line, int * richieste);
void offer_gen(struct merci * tipi_merci, int * offer_merci, int perc_offerte, int matr_line, int * offerte);

int riga_request = 0;
int riga_offer = 0;

void handler_start(int signal){
}

int main(int argc, char * argv[]){

    double harbor_pos_x;
    double harbor_pos_y;

    struct sigaction sa;
    int shm_fill_id, shm_merci_id, shm_pos_id, shm_richieste_id, shm_offerte_id;
    int sem_config_id, sem_offerte_richieste_id;
    int perc_richieste, perc_offerte;
    struct merci * tipi_merci;
    int * matr_richieste, * matr_offerte, * offerte, * richieste; 
    double * arr_pos;
    int i, matr_line;

    matr_richieste = malloc(sizeof(int) * SO_PORTI * (SO_MERCI + 1));
    matr_offerte = malloc(sizeof(int) * SO_PORTI * (SO_MERCI + 1));

    offerte = malloc(sizeof(int)*2);
    
    richieste = malloc(sizeof(int)*2);



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

    /**
     * Devo aspettare che il master termini la configurazione delle matrici delle offerte e delle richieste prima di continuare
    */

    sem_offerte_richieste_id = semget(getppid() + 1, 1, 0600 | IPC_CREAT);
    while(semctl(sem_offerte_richieste_id, 0, GETVAL) != 0);

    /**
     * Allocazione delle shared memory
    */

    shm_merci_id = shmget(getppid() + 1, sizeof(tipi_merci) * SO_MERCI, 0600 | IPC_CREAT);
    tipi_merci = shmat(shm_merci_id, NULL, 0);
    
    shm_richieste_id = shmget(getppid() + 2, sizeof(int) * SO_PORTI * (SO_MERCI + 1), 0600 | IPC_CREAT);
    matr_richieste = shmat(shm_richieste_id, NULL, 0);

    shm_offerte_id = shmget(getppid() + 3, sizeof(int) * SO_PORTI * (SO_MERCI + 1), 0600 | IPC_CREAT);
    matr_offerte = shmat(shm_offerte_id, NULL, 0);

    shm_pos_id = shmget(getppid() + 5, sizeof(double) * (SO_PORTI * 3), 0600 | IPC_CREAT);
    arr_pos = shmat(shm_pos_id, NULL, 0);

    perc_richieste = (rand() % 61) + 40;

    for(i = 0; i  < SO_PORTI;i++){
        if(matr_richieste[i*(SO_MERCI+1)] == getpid()){
            matr_line = i;
            break;
        }
    }

    request_gen(tipi_merci, matr_richieste, perc_richieste, matr_line, richieste);
    offer_gen(tipi_merci, matr_offerte, (100 - perc_richieste), matr_line, offerte);

    pause();


    /**
     * Creazione e allocazione in shared memory dell'array il cui contenuto corrisponderà alla posizione 
     * di ti tutti i porti all'interno della mappa
    */


    for(i = 0; i < SO_PORTI; i++){
        if(arr_pos[i * SO_PORTI] == getpid()){
            arr_pos[i * SO_PORTI + 1] = harbor_pos_x;
            arr_pos[i * SO_PORTI + 2] = harbor_pos_y;
        }
    }


}


void offer_gen(struct merci * tipi_merci, int * offer_merci, int perc_offerte, int matr_line, int * offerte){


}

void request_gen(struct merci * tipi_merci, int * request_merci, int perc_richieste, int matr_line, int * richieste){
    int gen_choiche, fill = 0;
    int i;
    int req_fill, id_merce;

    gen_choiche = rand() % 101;

    if(gen_choiche <= 70){
        req_fill = ((SO_FILL/SO_DAYS)/SO_PORTI) * (perc_richieste/100);
        printf("req_fill -> %d\n", req_fill);
        while(fill <= req_fill){
            id_merce = (rand() % (SO_MERCI+1)) + 1;
            if(fill + tipi_merci[id_merce].weight <= req_fill){
                richieste[riga_request * 2] = tipi_merci[id_merce].type;
                richieste[(riga_request * 2) + 1] = tipi_merci[id_merce].life;
                richieste = realloc(richieste, ((riga_request * 2 * sizeof(int)) + (2 * sizeof(int))));
                fill += tipi_merci[id_merce].weight;

                
                printf("fill -> %d, pid -> %d\n", fill, getpid());
                sleep(1);
            }
        }

        

        

    }


}
