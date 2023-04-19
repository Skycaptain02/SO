#include "env_var.h"
#include "../lib/ipc.h"

void request_gen(merci * tipi_merce, merci * merci_richieste_local);
void offer_gen(merci * tipi_merce, merci * merci_offerte_local);

int riga_request = 0;
int riga_offer = 0;

merci * tipi_merce;
merci * merci_richieste_local;
merci * merci_offerte_local;

void handler_start(int signal){
    switch(signal){
        case SIGUSR2:
            offer_gen(tipi_merce, merci_offerte_local);  
        break;
    }
}

int main(int argc, char * argv[]){

    double harbor_pos_x;
    double harbor_pos_y;

    struct sigaction sa;
    int shm_fill_id, shm_merci_id, shm_pos_id, shm_richieste_id, shm_offerte_id;
    int sem_config_id, sem_offerte_richieste_id;
    int perc_richieste, perc_offerte;
    int * matr_richieste, * matr_offerte;
    
    double * arr_pos;
    int i, matr_line;

    matr_richieste = malloc(sizeof(int) * SO_PORTI * (SO_MERCI + 1));
    matr_offerte = malloc(sizeof(int) * SO_PORTI * (SO_MERCI + 1));
    arr_pos = malloc(sizeof(double) * (SO_PORTI * 3));

    merci_richieste_local = malloc(100 * sizeof(merci));

    srand(getpid());
    
    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_start;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

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
        harbor_pos_y = (rand() % (SO_LATO + 1)) - (SO_LATO / 2);
    }

    /**
     * Una volta creati i porti carico in shared memory le loro posizioni
     * affinchè le navi possano rintracciarli in qualunque momento
    */

    shm_pos_id = shmget(getppid() + 4, sizeof(double) * (SO_PORTI * 3), 0600 | IPC_CREAT);
    arr_pos = shmat(shm_pos_id, NULL, 0);

    for(i = 0; i < SO_PORTI; i++){
        if(arr_pos[i * SO_PORTI] == getpid()){
            arr_pos[(i * SO_PORTI) + 1] = harbor_pos_x;
            arr_pos[(i * SO_PORTI) + 2] = harbor_pos_y;
            break;
        }
    }

    /**
     * Devo aspettare che il master termini la configurazione delle matrici delle offerte e delle richieste prima di continuare
    */

    sem_offerte_richieste_id = semget(getppid() + 1, 1, 0600 | IPC_CREAT);
    while(semctl(sem_offerte_richieste_id, 0, GETVAL) != 0);

    /**
     * Allocuazione delle shared memory
    */

    shm_merci_id = shmget(getppid() + 1, sizeof(tipi_merce) * SO_MERCI, 0600 | IPC_CREAT);
    tipi_merce = shmat(shm_merci_id, NULL, 0);
    
    shm_richieste_id = shmget(getppid() + 2, sizeof(int) * SO_PORTI * (SO_MERCI + 1), 0600 | IPC_CREAT);
    matr_richieste = shmat(shm_richieste_id, NULL, 0);

    shm_offerte_id = shmget(getppid() + 3, sizeof(int) * SO_PORTI * (SO_MERCI + 1), 0600 | IPC_CREAT);
    matr_offerte = shmat(shm_offerte_id, NULL, 0);

    /*request_gen(tipi_merce, matr_richieste, perc_richieste, matr_line, merci_richieste_local);*/
    /*offer_gen(tipi_merce, matr_offerte, (100 - perc_richieste), matr_line, offerte)*/

    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);
    sem_reserve(sem_config_id, 0);
    pause();
    while(1);
}


void offer_gen(merci * tipi_merce, merci * merci_offerte_local){
    printf("[%d] -> AO\n", getpid());
}

void request_gen(merci * tipi_merce, merci * merci_richieste_local){
    int gen_choiche;
    int fill = 0;
    int i;
    int req_fill, id_merce, perc_richieste;

    gen_choiche = rand() % 101;
    perc_richieste = (rand() % 61) + 40;

    if(gen_choiche <= 70){
        req_fill = ((SO_FILL/SO_DAYS)/SO_PORTI);
        while(1){
            id_merce = rand() % SO_MERCI;
            fill += tipi_merce[id_merce].weight;
            if(fill > req_fill){
                fill -= tipi_merce[id_merce].weight;
                break;
            }
            else{
                merci_richieste_local[riga_request].type = tipi_merce[id_merce].type;
                merci_richieste_local[riga_request].weight = tipi_merce[id_merce].weight;
                merci_richieste_local[riga_request].life = tipi_merce[id_merce].life;
                riga_request++;
            }
        }

        printf("Fill -> %d\n", fill);
    }
    


}
