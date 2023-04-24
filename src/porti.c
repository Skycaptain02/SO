#include "env_var.h"
#include "../lib/ipc.h"
#include "../lib/list.h"

node * request_offer_gen(merci * tipi_merce, node * merci_local, int * porti_selezionati, int * matrice, int percentuale);

merci * tipi_merce;
node * merci_richieste_local;
node * merci_offerte_local = NULL;

int flag_end = 0, flag_gen = 0;

void handler_start(int signal){
    switch(signal){
        case SIGUSR2:
            flag_gen = 1;
        break;
        case SIGABRT:
            flag_end = 1;
        break;
        default:
            printf("ERROR\n");
            break;
    }
}

int main(int argc, char * argv[]){

    double harbor_pos_x;
    double harbor_pos_y;

    struct sigaction sa;
    int shm_fill_id, shm_merci_id, shm_pos_id, shm_richieste_id, shm_offerte_id, shm_porti_selezionati_id;
    int sem_config_id, sem_offerte_richieste_id;
    int perc_richieste, perc_offerte, errno, banchine = SO_BANCHINE;
    int merci_scadute = 0;
    int * matr_richieste, * matr_offerte, * porti_selezionati;
    
    double * arr_pos;
    int i;

    srand(getpid());
    
    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_start;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);

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
        if(arr_pos[i * 3] == getpid()){
            arr_pos[(i * 3) + 1] = harbor_pos_x;
            arr_pos[(i * 3) + 2] = harbor_pos_y;
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

    shm_porti_selezionati_id = shmget(getppid() + 5, sizeof(int), 0600 | IPC_CREAT);
    porti_selezionati = shmat(shm_porti_selezionati_id, NULL, 0);

    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);
    sem_reserve(sem_config_id, 0);

    while(!flag_end){
        if(flag_gen){
            if(merci_offerte_local != NULL){
                merci_offerte_local = list_subtract(merci_offerte_local);
                merci_offerte_local = list_delete_zero(merci_offerte_local);
            }
            perc_richieste = (rand() % 21) + 40;
            merci_richieste_local = request_offer_gen(tipi_merce, merci_richieste_local, porti_selezionati, matr_richieste, perc_richieste);
            merci_offerte_local = request_offer_gen(tipi_merce, merci_offerte_local, porti_selezionati, matr_offerte, 100 - perc_richieste);
            flag_gen = 0;
        }
        else if(errno){
            printf("ERROR %s\n", strerror(errno));
        }
    }
    if(merci_offerte_local != NULL){
        merci_offerte_local = list_subtract(merci_offerte_local);
        merci_offerte_local = list_delete_zero(merci_offerte_local);
    }

    list_print(merci_offerte_local, getpid());
    
    if(merci_offerte_local != NULL){
        list_free(merci_offerte_local);
    }
    if(merci_richieste_local != NULL){
        list_free(merci_richieste_local);
    }
    
    shmdt(arr_pos);
    shmdt(tipi_merce);
    shmdt(matr_richieste);
    shmdt(matr_offerte);
}

node * request_offer_gen(merci * tipi_merce, node * merci_local, int * porti_selezionati, int * matrice, int percentuale){
    int fill = 0;
    int id_merce, riga_matr = 0;
    int i, flag_ctl = 1;
    double req_fill;

    for(i = 0; i < SO_PORTI; i++){
        if(getpid() == matrice[i*(SO_MERCI+1)]){
            riga_matr = i;
            break;
        }
    }
    req_fill = (((double)SO_FILL / (double)SO_DAYS) / (double)*porti_selezionati) * ((double)percentuale / 100); 
    if (SO_MERCI == 1)
    {    
        flag_ctl = 1;
        while(flag_ctl){
            if(matrice[(riga_matr * (2)) + 1] == 1){
                fill += tipi_merce[0].weight;
                if(fill > req_fill){
                    fill -= tipi_merce[0].weight;
                    flag_ctl = 0;
                    
                }
                else{
                    merci_local = list_insert(merci_local, tipi_merce[0]);
                }
            }
            else{
                flag_ctl = 0;
            }
        }   
    }
    else{
        while(1){
            flag_ctl = 1;
            id_merce = rand() % SO_MERCI;
            while(flag_ctl){
                if(matrice[riga_matr * (SO_MERCI + 1) + (id_merce + 1)] == 1){
                    flag_ctl = 0;
                }
                else{
                    id_merce = rand() % SO_MERCI;
                    flag_ctl = 1;
                }
            }
            fill += tipi_merce[id_merce].weight;
            if(fill > req_fill){
                fill -= tipi_merce[id_merce].weight;
                break;
            }
            else{
                merci_local = list_insert(merci_local, tipi_merce[id_merce]);
            }
        }
    }
    return merci_local;
}
