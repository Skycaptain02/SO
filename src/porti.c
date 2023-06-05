#include "env_var.h"
#include "../lib/ipc.h"
#include "../lib/list.h"

int SO_PORTI, SO_NAVI, SO_MERCI, SO_SIZE, SO_MIN_VITA, SO_MAX_VITA, SO_LATO, SO_SPEED, SO_CAPACITY, SO_BANCHINE; 
int SO_FILL, SO_LOADSPEED, SO_DAYS, SO_STORM_DURATION, SO_SWELL_DURATION, SO_MAELESTROM, PRINT_MERCI, CONVERSION_SEC_NSEN;

void readInputs();
void request_offer_gen(Merce *, int *, int * , int, int);
void dailyGen();
void swellPause();
void funcEnd();

List listaRichieste, listaOfferte;
Merce * tipi_merce;
sigset_t mask;
int * matr_richieste, * matr_offerte, * porti_selezionati;
int * arr_richieste_global, * arr_offerte_global;
int merci_spedite, merci_ricevute, numBanchine, sem_banchine_id;
int * qta_merci_scadute, * statusMerci, * maxOfferte, * maxRichieste, * offerteTot, * richiesteTot, * statusPorti;
int * tipi_richieste;
int * rem_life;
int gen;
int rigaStatus, rigaMatrice;
int flag_end = 0;
double * arr_pos;
int sem_opPorto_id;
int msg_porti_navi_id;


void handler_start(int signal){
    switch(signal){
        case SIGUSR1:
            dailyGen();
        break;
        case SIGABRT:
            flag_end = 1;
            funcEnd();
        break;
        case SIGUSR2:
            swellPause();
        break;
        case SIGTERM:
            if(listaOfferte.top != NULL){
                listSubtract(&listaOfferte, qta_merci_scadute, statusMerci, 1);
                /*La lista delle richieste scade? */
                /*listSubtract(&listaRichieste, qta_merci_scadute);*/
            }
        break;
        default:
        break;
    }
}

int main(int argc, char * argv[]){

    double harbor_pos_x;
    double harbor_pos_y;

    struct sigaction sa;
    int sem_config_id, sem_offerte_richieste_id;
    int perc_richieste, perc_offerte, errno;

    int shm_fill_id, shm_merci_id, shm_pos_id, shm_richieste_id, shm_offerte_id, shm_porti_selezionati_id, shm_statusMerci_id, shm_maxOfferte_id, shm_maxRichieste_id, shmStatusPorti_id;     
    int shm_richieste_global_id, shm_offerte_global_id, msg_porti_navi_id;
    
    int i, msg_bytes;
    struct MsgOp Operation;

    struct sembuf sem_op;

    readInputs();
    srand(getpid());
    
    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_start;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);


    listCreate(&listaRichieste);
    listCreate(&listaOfferte);

    tipi_richieste = malloc(sizeof(int) * SO_MERCI);
    qta_merci_scadute = malloc(sizeof(int));
    rem_life = malloc(sizeof(int));
    offerteTot = malloc(sizeof(int) * SO_MERCI);
    richiesteTot = malloc(sizeof(int) * SO_MERCI);

    /**
     * Creo i primi 4 porti su i 4 lati della mappa
     * La mappa e' stata configurata in modo che il centro di essa sia nelle coordinate x = 0; y = 0;
     * Come richiesto da consegna, i primi 4 porti sono collocati almeno in un lato della mappa, come posizione abbiamo scelte gli angoli del quadrato
    */

    sem_offerte_richieste_id = semget(getppid() + 1, 1, 0600 | IPC_CREAT);
    while(semctl(sem_offerte_richieste_id, 0, GETVAL) != 0);

    if(atoi(argv[1]) != 0){
        switch (atoi(argv[1])){
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
        if((unsigned int)arr_pos[i * 3] == getpid()){
            arr_pos[i * 3 + 1] = harbor_pos_x;
            arr_pos[i * 3 + 2] = harbor_pos_y;
            break;
        }
    }

    numBanchine = (rand() % SO_BANCHINE) + 1;

    /**
     * Allocuazione delle shared memory
    */

    shm_merci_id = shmget(getppid() + 1, sizeof(Merce) * SO_MERCI, 0600 | IPC_CREAT);
    tipi_merce = shmat(shm_merci_id, NULL, 0);
    
    shm_richieste_id = shmget(getppid() + 2, sizeof(int) * SO_PORTI * (SO_MERCI + 1), 0600 | IPC_CREAT);
    matr_richieste = shmat(shm_richieste_id, NULL, 0);

    shm_offerte_id = shmget(getppid() + 3, sizeof(int) * SO_PORTI * (SO_MERCI + 1), 0600 | IPC_CREAT);
    matr_offerte = shmat(shm_offerte_id, NULL, 0);

    shm_porti_selezionati_id = shmget(getppid() + 5, sizeof(int), 0600 | IPC_CREAT);
    porti_selezionati = shmat(shm_porti_selezionati_id, NULL, 0);

    shm_richieste_global_id = shmget(getppid() + 6, sizeof(int) * ((SO_MERCI+1)*SO_PORTI), 0600 | IPC_CREAT);
    arr_richieste_global = shmat(shm_richieste_global_id, NULL, 0);

    shm_offerte_global_id = shmget(getppid() + 7, sizeof(int) * ((SO_MERCI+1)*SO_PORTI), 0600 | IPC_CREAT);
    arr_offerte_global = shmat(shm_offerte_global_id, NULL, 0);

    shm_statusMerci_id = shmget(getppid() + 10, sizeof(int) * (SO_MERCI) * 5, 0600 | IPC_CREAT);
    statusMerci = shmat(shm_statusMerci_id, NULL, 0);

    shm_maxOfferte_id = shmget(getppid() + 11, sizeof(int) * SO_MERCI * 2, 0600 | IPC_CREAT);
    maxOfferte = shmat(shm_maxOfferte_id, NULL, 0);

    shm_maxRichieste_id = shmget(getppid() + 12, sizeof(int) * SO_MERCI * 2, 0600 | IPC_CREAT);
    maxRichieste = shmat(shm_maxRichieste_id, NULL, 0);

    shmStatusPorti_id = shmget(getppid() + 15, sizeof(int) * SO_PORTI * 6, 0600 | IPC_CREAT);
    statusPorti = shmat(shmStatusPorti_id, NULL, 0);
    /*Fine allocazione shared memory*/

    /**
     * Allocazione semafori
    */

    sem_banchine_id = semget(getpid(), 1, 0600 | IPC_CREAT);
    semctl(sem_banchine_id, 0, SETVAL , numBanchine);

    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);

    sem_opPorto_id =semget(getpid(), 2, 0600 | IPC_CREAT);
    semctl(sem_opPorto_id, 0, SETVAL, 1);
    semctl(sem_opPorto_id, 1, SETVAL, 1);
    /*Fine allocazione semafori*/

    /*Allocazione coda di messaggi*/
    msg_porti_navi_id = msgget(getppid() , 0600 | IPC_CREAT);
    /*Fine allocazione coda di messaggi*/

    i = 0;
    while(arr_richieste_global[i * (SO_MERCI + 1)] != getpid()){
        i++;
    }
    rigaMatrice = i;
    i = 0;
    while(statusPorti[i * 6] != getpid()){
        i++;
    }
    rigaStatus = i;
    i = 0;

    statusPorti[(rigaStatus * 6) + 5] = numBanchine;
    statusPorti[(rigaStatus * 6) + 4] = semctl(sem_banchine_id, 0, GETVAL);

    /**
     * Finita la configuarazione iniziale, abbasso il semaforo per comunicare al master che il porto e' pronto a ricevere il segnale SIGUSR1 per iniziare la generazione delle Merce
    */

    sem_reserve(sem_config_id, 0);

    while(!flag_end){
        msg_bytes = msgrcv(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), getpid(), 0);
        if(msg_bytes >= 0){
            switch (Operation.operation){
                case 0:
                    if(semctl(sem_banchine_id, 0, GETVAL) > 0){
                        sem_reserve(sem_banchine_id,0);
                        statusPorti[(rigaStatus * 6) + 4] = semctl(sem_banchine_id, 0, GETVAL);
                        Operation.extra = 0;
                        Operation.type = (unsigned int)Operation.pid_nave;
                        Operation.operation = 0;
                        msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2  + sizeof(pid_t), 0);
                    }
                    else{
                        Operation.operation = -1;
                        Operation.type = (unsigned int)Operation.pid_nave;
                        msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2  + sizeof(pid_t), 0);                              
                    }
                break;
                
                case 1:
                    listRemoveToLeft(&listaRichieste, NULL, Operation.extra);
                    statusMerci[((Operation.extra - 1) * 5) + 2] += 1; /*Inserisco al porto*/
                    statusMerci[((Operation.extra - 1) * 5) + 1] -= 1; /*Tolgo su nave*/
                    Operation.type = (unsigned int)Operation.pid_nave;
                    Operation.extra = 0;
                    Operation.operation = 1;
                    msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), 0);
                    statusPorti[(rigaStatus * 6) + 2] += 1;
                break;

                case 2:
                    Operation.operation = 3;
                    Operation.type = (unsigned int)Operation.pid_nave;
                    listRemoveToLeft(&listaOfferte, rem_life, Operation.extra);
                    statusMerci[((Operation.extra - 1) * 5)] -= 1;     /*Tolgo al porto*/
                    statusMerci[((Operation.extra - 1) * 5) + 1] += 1; /*Inserisco su nave*/
                    Operation.extra = * rem_life;
                    msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2  + sizeof(pid_t), 0);
                    statusPorti[(rigaStatus * 6) + 3] += 1;
                break;

                case 4:
                    sem_release(sem_banchine_id, 0);
                    statusPorti[(rigaStatus * 6) + 4] = semctl(sem_banchine_id, 0, GETVAL);
                    Operation.type = (unsigned int)Operation.pid_nave;
                    Operation.extra = 0;
                    Operation.operation = - 1;
                    msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), 0);
                break;
            }
        }
    }
}

/**
 * Questa funzione andrà a generara una lista delle offerte o delle richieste, a seconda di chi l'ha richiamata
 * 1) Capisce tramite pid in quale riga della matrice delle richieste/offerte il porto si trova
 * 2) Sapendo in che riga si trova continua a generare un indice di una merce a caso finché non ne trova uno che possa effettivamente generare
 * 3) Trovata la merce da creare ne va a controllare il peso, se l'inserimento della merce causerebbe un eccedimento della capacità effettiva del porto nella giornata attuale non la inserisco
*/

void request_offer_gen(Merce * tipi_merce, int * porti_selezionati, int * matrice, int percentuale, int flag){
    int fill = 0;
    int id_merce = 1;
    int i, flag_ctl = 1, exit = 0;
    double req_fill;
    
    /**
     * Dividiamo il fill giornaliero per il numero di porti selezionati per quel giorno (min 4), e suddividiamo il tutto tramite percentuale
     * in fill per l'offerta e fill per la richiesta (Es 40% di fill per richieste e 60% di fill per le offerte)
    */
   
    req_fill = (((double)SO_FILL / (double)SO_DAYS) / (double)* porti_selezionati) * ((double)percentuale / 100); 
    

    /*Caso specifico in cui SO_MERCI e' impostato a 1*/
    if (SO_MERCI == 1){    
        flag_ctl = 1;
        while(flag_ctl){
            if(matrice[(rigaMatrice * (2)) + 1] == 1){
                fill += tipi_merce[0].weight;
                if(fill > req_fill){
                    fill -= tipi_merce[0].weight;
                    flag_ctl = 0;  
                }
                else{
                    if(flag){
                        offerteTot[0] += 1;
                        arr_offerte_global[(rigaMatrice * (SO_MERCI + 1)) + 1] += 1;
                        listInsert(&listaOfferte, tipi_merce[0]);
                        statusMerci[((0) * 5)] += 1;
                        
                    }else{
                        richiesteTot[0] +=1;
                        arr_richieste_global[(rigaMatrice * (SO_MERCI+1)) + 1] += 1;
                        listInsert(&listaRichieste, tipi_merce[0]);
                    }
                }
            }
            else{
                flag_ctl = 0;
            }
        }   
    }
    else{  
        while(fill <= req_fill){
            flag_ctl = 1;
            while(flag_ctl){
                if(matrice[rigaMatrice * (SO_MERCI + 1) + (id_merce)] == 1){
                    flag_ctl = 0;
                }
                else{
                    id_merce = (id_merce != SO_MERCI) ? id_merce += 1 : 1;
                    flag_ctl = 1;
                }
            }
            fill += tipi_merce[id_merce-1].weight;
            if(fill <= req_fill){
                if(flag){/*OFFERTE*/
                    offerteTot[(id_merce - 1)] += 1;
                    arr_offerte_global[(rigaMatrice * (SO_MERCI + 1)) + id_merce] += 1;
                    listInsert(&listaOfferte, tipi_merce[id_merce - 1]);
                    statusMerci[(id_merce - 1) * 5] += 1;
                }
                else{/*RICHIESTE*/
                    richiesteTot[(id_merce - 1)] += 1;
                    arr_richieste_global[(rigaMatrice * (SO_MERCI + 1)) + id_merce] += 1;
                    listInsert(&listaRichieste, tipi_merce[id_merce - 1]);
                    
                }
                
            }
            id_merce = (id_merce != SO_MERCI) ? id_merce += 1 : 1;
        } 
    }
    for(i = 0; i < SO_MERCI; i++){
        if(flag){
            if(offerteTot[i] > maxOfferte[(i * 2) + 1]){
                maxOfferte[i * 2] = getpid();
                maxOfferte[(i * 2) + 1] = offerteTot[i];
            }
        }
        else{
            if(richiesteTot[i] > maxRichieste[(i * 2) + 1]){
                maxRichieste[i * 2] = getpid();
                maxRichieste[(i * 2) + 1] = richiesteTot[i];
            }
            
        }
    }
   
}

/**
 * Ogni giorno il porto, se scelto,  riceve un segnale dal master il quale gli comomunica che nella giornata attuale tale porto deve
 * attivare le procedure di generazione delle richieste e delle offerte all'interno delle proprie liste
 * Offerte e richieste vengono generate a percentuale randomica ogni giorno, il porto "decide" che percentuale assegnare a entrambe
*/

void dailyGen(){
    int perc_richieste, i = 0;
    perc_richieste = (rand() % 21) + 40;

    if(listaOfferte.top != NULL){
        listSubtract(&listaOfferte, qta_merci_scadute, statusMerci, 1);
        /*La lista delle richieste scade? */
        /*listSubtract(&listaRichieste, qta_merci_scadute);*/
    }
    request_offer_gen(tipi_merce, porti_selezionati, matr_richieste, perc_richieste, 0);
    request_offer_gen(tipi_merce, porti_selezionati, matr_offerte, 100 - perc_richieste, 1);

    statusPorti[(rigaStatus * 6) + 1] = listLength(&listaOfferte);
}

/**
 * Funzione che entra in esecuzione solo se il porto è stato bersagliato da una mareggiata
*/

void swellPause(){
    struct timespec req, rem;
    int modulo;
    int i = 0;
    int exit = 0;
    double nsec, waitTime;
    sigset_t maskBlock;
    rem.tv_nsec = 0;
    rem.tv_sec = 0;

    sigemptyset(&maskBlock);
    sigaddset(&maskBlock, SIGTERM);
    sigaddset(&maskBlock, SIGUSR1);
    sigaddset(&maskBlock, SIGUSR2);
    sigprocmask(SIG_BLOCK, &maskBlock, NULL);
    if(SO_SWELL_DURATION < 24){
        modulo = 0;
        nsec = (((double)SO_SWELL_DURATION / (double)24) * CONVERSION_SEC_NSEN);
        req.tv_sec = (time_t)(modulo);
        req.tv_nsec = (long)nsec;
    }
    else{
        modulo = (int)SO_SWELL_DURATION / 24;
        nsec = ((double)SO_SWELL_DURATION / (double)24 - modulo) * CONVERSION_SEC_NSEN;
        req.tv_sec = (time_t)(modulo);
        req.tv_nsec = (long)(nsec);
    }
    
    nanosleep(&req, &rem);

    sigemptyset(&maskBlock);
    sigaddset(&maskBlock, SIGTERM);
    sigaddset(&maskBlock, SIGUSR1);
    sigaddset(&maskBlock, SIGUSR2);
    sigprocmask(SIG_UNBLOCK, &maskBlock, NULL);
}

void readInputs(){
    const char * file_path = "../src/env_var.txt";
    char buffer[1024];
    char buffer_cpy[1024];
    ssize_t bytes_read;
    char * token;
    FILE * file_descriptor = fopen(file_path, "r");
    

    if (!file_descriptor) {
        perror("Failed to open the file");
    }

    while (fgets(buffer, 1024, file_descriptor)){
        token = strtok(buffer, " ");
        if(strcmp(token, "SO_NAVI") == 0){
            token = strtok(NULL, " ");
            SO_NAVI = atoi(token);
        }else if(strcmp(token, "SO_PORTI") == 0){
            token = strtok(NULL, " ");
            SO_PORTI = atoi(token);
        }else if(strcmp(token, "SO_MERCI") == 0){
            token = strtok(NULL, " ");
            SO_MERCI = atoi(token);
        }else if(strcmp(token, "SO_SIZE") == 0){
            token = strtok(NULL, " ");
            SO_SIZE = atoi(token);
        }else if(strcmp(token, "SO_MIN_VITA") == 0){
            token = strtok(NULL, " ");
            SO_MIN_VITA = atoi(token);
        }else if(strcmp(token, "SO_MAX_VITA") == 0){
            token = strtok(NULL, " ");
            SO_MAX_VITA = atoi(token);
        }else if(strcmp(token, "SO_LATO") == 0){
            token = strtok(NULL, " ");
            SO_LATO = atoi(token);
        }else if(strcmp(token, "SO_SPEED") == 0){
            token = strtok(NULL, " ");
            SO_SPEED = atoi(token);
        }else if(strcmp(token, "SO_CAPACITY") == 0){
            token = strtok(NULL, " ");
            SO_CAPACITY = atoi(token);
        }else if(strcmp(token, "SO_BANCHINE") == 0){
            token = strtok(NULL, " ");
            SO_BANCHINE = atoi(token);
        }else if(strcmp(token, "SO_FILL") == 0){
            token = strtok(NULL, " ");
            SO_FILL = atoi(token);
        }else if(strcmp(token, "SO_LOADSPEED") == 0){
            token = strtok(NULL, " ");
            SO_LOADSPEED = atoi(token);
        }else if(strcmp(token, "SO_DAYS") == 0){
            token = strtok(NULL, " ");
            SO_DAYS = atoi(token);
        }else if(strcmp(token, "SO_STORM_DURATION") == 0){
            token = strtok(NULL, " ");
            SO_STORM_DURATION = atoi(token);
        }else if(strcmp(token, "SO_SWELL_DURATION") == 0){
            token = strtok(NULL, " ");
            SO_SWELL_DURATION = atoi(token);
        }else if(strcmp(token, "SO_MAELESTROM") == 0){
            token = strtok(NULL, " ");
            SO_MAELESTROM = atoi(token);
        }else if(strcmp(token, "PRINT_MERCI") == 0){
            token = strtok(NULL, " ");
            PRINT_MERCI = atoi(token);
        }else if(strcmp(token, "CONVERSION_SEC_NSEN") == 0){
            token = strtok(NULL, " ");
            CONVERSION_SEC_NSEN = atoi(token);
        }
    }
    fclose(file_descriptor);
}

void funcEnd(){
    listFree(&listaRichieste);
    listFree(&listaOfferte);

    shmdt(arr_pos);
    shmdt(tipi_merce);
    shmdt(matr_richieste);
    shmdt(matr_offerte);
    shmdt(porti_selezionati);
    shmdt(arr_richieste_global);
    shmdt(arr_offerte_global);
    shmdt(statusMerci);
    shmdt(maxOfferte);
    shmdt(maxRichieste);
    shmdt(statusPorti);

    semop(sem_banchine_id, NULL, IPC_RMID);
    semop(sem_opPorto_id, NULL, IPC_RMID);

    msgctl(msg_porti_navi_id, IPC_RMID, NULL);

    free(tipi_richieste);
    free(qta_merci_scadute);
    free(rem_life);
    free(offerteTot);
    free(richiesteTot);

    exit(0);
}