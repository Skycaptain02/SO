#include "env_var.h"
#include "../lib/ipc.h"
#include "../lib/list.h"

node * request_offer_gen(merci * tipi_merce, node * merci_local, int * porti_selezionati, int * matrice, int percentuale);
void daily_gen();

merci * tipi_merce;
node * merci_richieste_local;
node * merci_offerte_local;
int * matr_richieste, * matr_offerte, * porti_selezionati;

int flag_end = 0, flag_gen = 0;

void handler_start(int signal){
    switch(signal){
        case SIGUSR2:
            daily_gen();
        break;
        case SIGABRT:
            flag_end = 1;
        break;
        default:
        break;
    }
}

int main(int argc, char * argv[]){

    double harbor_pos_x;
    double harbor_pos_y;

    struct sigaction sa;
    int shm_fill_id, shm_merci_id, shm_pos_id, shm_richieste_id, shm_offerte_id, shm_porti_selezionati_id, msg_porti_navi_id;
    int sem_config_id, sem_offerte_richieste_id;
    int perc_richieste, perc_offerte, errno, banchine = SO_BANCHINE;
    int merci_scadute = 0;
    
    
    double * arr_pos;
    int i, msg_bytes;
    struct msgOp Operation;

    int * tipi_richieste;

    srand(getpid());
    
    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_start;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);

    tipi_richieste = malloc(sizeof(int)*SO_MERCI);

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

    /**
     * Finita la configuarazione iniziale, abbasso il semaforo per comunicare al master che il porto e' pronto a ricevere il segnale SIGUSR1 per iniziare la generazione delle merci
    */

    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);
    sem_reserve(sem_config_id, 0);

    msg_porti_navi_id = msgget(getppid() , 0600 | IPC_CREAT);

    /**
     * Qui il porto andrà a generare la propria lista delle richieste e delle offerte giorno dopo giorno decrementato il parametro LIFE delle merci di 1 ogni giorno che passa
     * Il peso totale tra offerte e richieste e' (SO_FILL/SO_DAYS/porti_selezionati) * perc_richieste
     * La prima parte della moltiplicazione ci garantisce che tutto SO_FILL peso verrà generato al termine della simulazione
     * La seconda parte e' una divisione del peso delle merci che un porto può generare tra richieste e offerte in modo da dividere tale quantità in due parti
    */

    while(!flag_end){
        msg_bytes = msgrcv(msg_porti_navi_id, &Operation, sizeof(int), getpid(), 0);
        if(msg_bytes >= 0){
            if(banchine > 0){
                Operation.operation = 0;
            }
            else{
                Operation.operation = -1;
            }
            msgsnd(msg_porti_navi_id, &Operation, sizeof(int),0);
        }
        
        
    }
    if(merci_offerte_local != NULL){
        merci_offerte_local = list_subtract(merci_offerte_local);
        merci_offerte_local = list_delete_zero(merci_offerte_local);
    }
    

    /**
     * Simulazione finta, ricevuto segnale di SIGABRT, deallocazione shared memory e liste offerte e richieste
    */
    
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

/**
 * Questa funzione andrà a generara una lista delle offerte o delle richieste, a seconda di chi l'ha richiamata
 * 1) Capisce tramite pid in quale riga della matrice delle richieste/offerte il porto si trova
 * 2) Sapendo in che riga si trova continua a generare un indice di una merce a caso finché non ne trova uno che possa effettivamente generare
 * 3) Trovata la merce da creare ne va a controllare il peso, se l'inserimento della merce causerebbe un eccedimento della capacità effettiva del porto nella giornata attuale non la inserisco
*/

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

    /**
     * Caso specifico in cui SO_MERCI e' impostato a 1
    */
   
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

void daily_gen(){
    int perc_richieste;
    int shm_arr_richieste_id, shm_arr_offerte_id;
    int * arr_richieste_local, * arr_offerte_local;

    if(merci_offerte_local != NULL){
        merci_offerte_local = list_subtract(merci_offerte_local);
        merci_offerte_local = list_delete_zero(merci_offerte_local);
    }
    perc_richieste = (rand() % 21) + 40;
    merci_richieste_local = request_offer_gen(tipi_merce, merci_richieste_local, porti_selezionati, matr_richieste, perc_richieste);
    merci_offerte_local = request_offer_gen(tipi_merce, merci_offerte_local, porti_selezionati, matr_offerte, 100 - perc_richieste);
    flag_gen = 0;

    shm_arr_richieste_id = shmget(getpid(), sizeof(merci) * list_length(merci_richieste_local), 0600 | IPC_CREAT);
    arr_richieste_local = shmat(shm_arr_richieste_id, NULL, 0);
    shmctl(shm_arr_richieste_id, IPC_RMID, NULL);

    shm_arr_offerte_id = shmget(getpid() + getppid(), sizeof(merci) * list_length(merci_offerte_local), 0600 | IPC_CREAT);
    arr_offerte_local = shmat(shm_arr_offerte_id, NULL, 0);
    shmctl(shm_arr_offerte_id, IPC_RMID, NULL);
}