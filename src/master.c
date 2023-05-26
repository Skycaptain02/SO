#include "env_var.h"
#include "../lib/ipc.h"

int flagEndMaelstrom = 0;
int checkEndOffers = 0, checkEndRequests = 0, flagEndMaterials = 1;

void gen_richiesta_offerta(int * pidPorti, int * arr_richieste, int * arr_offerte, int print);
void gen_offerta(int matr_richieste[SO_PORTI][SO_MERCI+1], int matr_offerte[SO_PORTI][SO_MERCI+1], int * pidPorti, int num_merci, int print);
void check_inputs();
void dailyPrint(int *, int *, int *, int *, int);
void finalReport(int *, int *, int *, int *, int *, int *, int *);
void endSimulation(pid_t *, pid_t *, pid_t);

void handler(int signal){
    switch (signal){
        case SIGABRT:
        printf("TUTTE NAV MORT\n");
            flagEndMaelstrom = 1;
        break;
    default:
        break;
    }
}

int main(int argc, char * argv[]){

    int shm_porti_selezionati_id, shm_pos_porti_id, shm_merci_id, shm_richieste_id, shm_offerte_id, shm_richieste_global_id, shm_offerte_global_id, shm_merci_cosegnate_id, shm_statusPorti_id;
    int shm_statusNavi_id, shm_statusMerci_id, shm_statusMerciTot_id, shm_maxOfferte_id, shm_maxRichieste_id, shm_pidNavi_id, shm_pidPorti_id, shm_portiSwell_id;
    int * arr_richieste, * arr_offerte, * arr_richieste_global, * arr_offerte_global, * statusMerciTot, * maxOfferte, * maxRichieste, * statusPorti, * portiSwell;
    double * arr_pos_porti;
    int * porti_selezionati, * merci_consegnate, * statusNavi, * statusMerci, * shmPidNavi, * shmPidPorti;
    Merce * tipi_merci;

    sigset_t mask_block;


    struct sigaction sa;
    int i, j, k, z, x, y, status, errno;
    

    
    int sem_config_id = 0, sem_offerte_richieste_id = 0;
    int * porti_random, * random_index;

    pid_t * pidNavi, * pidPorti, pid_meteo;
   
    int porto_scelto; 
    int flag = 1;
    char argv_buffer[50];

    char * args_navi[]  = {"navi.c", NULL};
    char * args_porti[] = {"porti.c", NULL};
    char * args_meteo[] = {"meteo.c", NULL};
    char * args_merci[] = {"merci.c", NULL};

    struct timespec req, rem;
    
    pidNavi = malloc(sizeof(pidNavi) * SO_NAVI);
    pidPorti = malloc(sizeof(pidPorti) * SO_PORTI);

    sigemptyset(&mask_block);
    sigaddset(&mask_block, SIGINT);
    sigprocmask(SIG_BLOCK, &mask_block, NULL);

    srand(getpid());

      
    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGABRT, &sa, NULL);

    check_inputs();

    /**
     * SHARED MEMORY
     * 
     * parent pid + 1: lista Merce per simulazione
     * parent pid + 2: matrice indicante quali Merce possono essere richieste da ciascun porto
     * parend pid + 3: matrice indicante quali offerte possono essere richieste da ciascun porto
     * parent pid + 4: posizione porti
     * parent pid + 5: gestione generazione offerte e richieste dai porti
     * parent pid + 6: matrice di tutte le Merce richieste da tutti i porti
     * parent pid + 7: matrice di tutte le Merce offerte da tutti i poorti
     * parent pid + 8: array che conta il quantitativo di merci consegnate da tutte le navi
     * parent pid + 9: array che visualizza lo status delle navi nel corso della simulazione
     * parent pid + 10: array che salva tutti gli status delle merci in corso della simulazione
     * parent pid + 11: array che va a salvare i porti con l'offerta maggiore di una specifica merce
     * parent pid + 12: array che va a salvare i porti che hanno effettuato il maggior numero di richieste
     * parent pid + 13: array che contiene i pid delle navi
     * parent pid + 14: array che contiene i pid dei porti
     * parent pid + 15: array il quale contiene le statistiche merci all'interno di uno specifico porto
     * parent pid + 16: array dei porti che sono stati colpiti dalla mareggiata
    */

    shm_merci_id = shmget(getpid() + 1, sizeof(Merce) * SO_MERCI, 0600 | IPC_CREAT);
    tipi_merci = shmat(shm_merci_id, NULL, 0);
    
    shm_richieste_id = shmget(getpid() + 2, sizeof(int) * ((SO_MERCI + 1)*SO_PORTI), 0600 | IPC_CREAT);
    arr_richieste = shmat(shm_richieste_id, NULL, 0);

    shm_offerte_id = shmget(getpid() + 3, sizeof(int) * ((SO_MERCI + 1) * SO_PORTI), 0600 | IPC_CREAT);
    arr_offerte = shmat(shm_offerte_id, NULL, 0);

    shm_pos_porti_id = shmget(getpid() + 4, sizeof(double) * (SO_PORTI * 3), 0600 | IPC_CREAT);
    arr_pos_porti = shmat(shm_pos_porti_id, NULL, 0);

    shm_porti_selezionati_id = shmget(getpid() + 5, sizeof(int), 0600 | IPC_CREAT);
    porti_selezionati = shmat(shm_porti_selezionati_id, NULL, 0);

    shm_richieste_global_id = shmget(getpid() + 6, sizeof(int) * ((SO_MERCI + 1) * SO_PORTI), 0600 | IPC_CREAT);
    arr_richieste_global = shmat(shm_richieste_global_id, NULL, 0);

    shm_offerte_global_id = shmget(getpid() + 7, sizeof(int) * ((SO_MERCI + 1) * SO_PORTI), 0600 | IPC_CREAT);
    arr_offerte_global = shmat(shm_offerte_global_id, NULL, 0);

    shm_merci_cosegnate_id = shmget(getpid() + 8, sizeof(int) * SO_MERCI, 0600 | IPC_CREAT);
    merci_consegnate = shmat(shm_merci_cosegnate_id, NULL, 0);

    shm_statusNavi_id = shmget(getpid() + 9, sizeof(int) * SO_NAVI * 6, 0600 | IPC_CREAT);
    statusNavi = shmat(shm_statusNavi_id, NULL, 0);

    shm_statusMerci_id = shmget(getpid() + 10, sizeof(int) * (SO_MERCI) * 5, 0600 | IPC_CREAT);
    statusMerci = shmat(shm_statusMerci_id, NULL, 0);

    shm_maxOfferte_id = shmget(getpid() + 11, sizeof(int) * SO_MERCI * 2, 0600 | IPC_CREAT);
    maxOfferte = shmat(shm_maxOfferte_id, NULL, 0);

    shm_maxRichieste_id = shmget(getpid() + 12, sizeof(int) * SO_MERCI * 2, 0600 | IPC_CREAT);
    maxRichieste = shmat(shm_maxRichieste_id, NULL, 0);

    shm_pidNavi_id = shmget(getpid() + 13, sizeof(pid_t) * SO_NAVI, 0600 | IPC_CREAT);
    shmPidNavi = shmat(shm_pidNavi_id, NULL, 0);

    shm_pidPorti_id = shmget(getpid() + 14, sizeof(pid_t) * SO_PORTI, 0600 | IPC_CREAT);
    shmPidPorti = shmat(shm_pidPorti_id, NULL, 0);

    shm_statusPorti_id = shmget(getpid() + 15, sizeof(int) * SO_PORTI * 6, 0600 | IPC_CREAT);
    statusPorti = shmat(shm_statusPorti_id, NULL, 0);

    shm_portiSwell_id = shmget(getpid() + 16, sizeof(int) * SO_DAYS, 0600 | IPC_CREAT);
    portiSwell = shmat(shm_portiSwell_id, NULL, 0);
    /*Fine allocazione delle shared memory*/

    /**
     * SEMAFORI
     * 
     * parent pid + 0: configuarazione iniziale
     * parent pid + 1: creazione richieste/offerte
    */

    sem_config_id = semget(getpid(), 1, 0600 | IPC_CREAT);
    sem_set_val(sem_config_id, 0, (SO_NAVI + SO_PORTI + 1));

    sem_offerte_richieste_id = semget(getpid() + 1, 1, 0600 | IPC_CREAT);
    sem_set_val(sem_offerte_richieste_id, 0, 1);
    /*Fine Sezione crezione semaforo per configuazione*/

    for(i = 0; i < SO_MERCI; i++){
        /*merci_consegnate[i] = 0;*/
    }

    switch(fork()){
        case - 1:
            printf("C'è stato un errore nel fork per le merce %s", strerror(errno));
            exit(- 1);
        break;
        case 0:
            execve("../bin/merci", args_merci , NULL);
            exit(0);
        break;
        default:
        break;
    }

    while(wait(NULL) != - 1);

    for(i = 0; i < SO_MERCI * 5; i++){
        statusMerci[i] = 0;
    }

    /**
     * Generazione dei primi 4 porti su ogni lato della mappa
    */

    for(i = 0; i < 4; i++){
        switch (pidPorti[i] = fork()){
            case -1:
                printf("C'è stato un errore nel fork per i porti: %s", strerror(errno));
                exit(-1);
            case 0:
                sprintf(argv_buffer, "%d", (i+1));
                args_porti[1] = argv_buffer;
                execve("../bin/porti", args_porti, NULL);
                exit(0); 
                break;
            default:
            break;
        }
    }
    
    args_porti[1] = " ";
    
    /**
     * Creazione restanti porti
    */
  
    for(i = 4; i < SO_PORTI; i++){
        switch (pidPorti[i] = fork())
        {
            case -1:
                printf("C'è stato un errore nel fork per i porti: %s", strerror(errno));
                exit(-1);
                break;
            case 0:
                execve("../bin/porti", args_porti , NULL);
                exit(0);
                break;
            default:
            break;
        }
    }

    for(i = 0; i < SO_PORTI; i++){
        arr_pos_porti[i * 3] = (unsigned int)pidPorti[i];
        statusPorti[i * 6] = (unsigned int)pidPorti[i];
        arr_offerte_global[i * (SO_MERCI + 1)] = (unsigned int)pidPorti[i];
        arr_richieste_global[i * (SO_MERCI + 1)] = (unsigned int)pidPorti[i];
    }


    /**
     * Dopo aver generato tutti i processi porti, vado a creare le matrici delle richieste e delle offerte, creade un semaforo che comunicherà
     * ai porti quando iniziare a generare le effettive risorse all'interno del loro magazzino
    */

    printf("[SISTEMA]\t -> \t TUTTI I PORTI SONO PRONTI\n");

    gen_richiesta_offerta(pidPorti, arr_richieste, arr_offerte, 0);
    sem_reserve(sem_offerte_richieste_id, 0);
    
    /**
     * Generazionie di tutte le navi
    */
    
    for(i = 0; i < SO_NAVI; i++){
        switch (pidNavi[i] = fork()){
            case -1:
                printf("C'è *stato un errore nel fork per le navi: %s", strerror(errno));
                exit(-1);
                break;
            case 0:
                execve("../bin/navi", args_navi , NULL);
                exit(- 1);
            break;
            default:
            break;
        }
    }

    for(i = 0; i < SO_NAVI; i++){
        statusNavi[i * 6] = (unsigned int)pidNavi[i];
        statusNavi[(i * 6) + 1] = 1;
    }

    printf("[SISTEMA]\t -> \t TUTTE LE NAVI SONO PRONTE\n");

    /**
     * Generazione processo meteo
    */
    
    for(i = 0; i < SO_NAVI; i++){
        shmPidNavi[i] = pidNavi[i];
    }
    for(i = 0; i < SO_PORTI; i++){
        shmPidPorti[i] = pidPorti[i];
    }
 
    switch(pid_meteo = fork()){
        case -1:
            printf("C'è stato un errore nel fork per il meteo: %s", strerror(errno));
            exit(-1);
            break;
        case 0:
            execve("../bin/meteo", args_meteo , NULL);
            exit(0);
        default:
        break;
    }

    printf("[SISTEMA]\t -> \t METEO PRONTO\n");

    /**
     * La simulazione effettiva comincia qui
     * All'inizio del ciclo while vengono scelti un numero casuale si porti i quali dovranno generare risorse all'interno del loro magazzino
     * Tali porti verranno scelti a caso, non ripetuti, in modo che non si verichi un possibile merge di segnali
     * I porti che giorno N sono stati scelti riceveranno dal master un segnale SIGUSR2 il quale comunicherà ai porti di creare le proprie Merce
    */

    i = 0;
    k = 0;
    printf("[SISTEMA]\t -> \t INIZIO SIMULAZIONE\n");
    while(semctl(sem_config_id, 0, GETVAL) != 0);
    while(i != SO_DAYS && flagEndMaterials && !flagEndMaelstrom){
        checkEndOffers = 0;
        checkEndRequests = 0;
        * porti_selezionati = (rand() % (SO_PORTI - 3)) + 4;
        porto_scelto = rand() % SO_PORTI;
        if(i == 0){
            random_index = malloc(sizeof(int) * SO_PORTI);
            porti_random = malloc(sizeof(int) * (* porti_selezionati));
        }
        else{
           porti_random = malloc(sizeof(int) * (* porti_selezionati));
        }

        j = 0;
        while(j != SO_PORTI){
            random_index[j] = j;
            if(k < * porti_selezionati){
                porti_random[k] = -1;
                k++;
            }
            j++;
        }

        for(k = 0; k < * porti_selezionati; k++){
            flag = 1;
            while(flag){
                if(random_index[porto_scelto] != -1 && porti_random[k] == -1){
                    porti_random[k] = random_index[porto_scelto];
                    random_index[porto_scelto] = -1;
                    flag = 0;
                }
                else{
                    porto_scelto = rand() % SO_PORTI;
                }
            }
        }
        i++;
        for(j = 0; j < * porti_selezionati; j++){
            kill(pidPorti[porti_random[j]], SIGUSR1);
        }
        kill(pid_meteo, SIGUSR1);

        j = 0;
        k = 0;

        req.tv_sec = 1;
        req.tv_nsec = 0;
        nanosleep(&req, &rem);
        dailyPrint(statusNavi, statusMerci, statusPorti, portiSwell, i);
        for(x = 1; x <= SO_MERCI; x++){
            for(y = 0; y < SO_PORTI; y++){
                checkEndOffers += arr_offerte_global[y * (SO_MERCI + 1) + x];
                checkEndRequests += arr_richieste_global[y * (SO_MERCI + 1) + x];
            }
        }
        if(checkEndOffers == 0 || checkEndRequests == 0){
            flagEndMaterials = 0;
        }
        
        free(porti_random);
        
    }
    /*
    for(i = 0; i < SO_MERCI; i++){
        printf("Merce-> %d\tconsegnata: %d\tvolte\n", i+1, merci_consegnate[i]);
    }
    */
    
    /**
     * Fine sumulazione, invio un SIGABRT a tutti i processi indicandogli di terminare, subito dopo dealloco tutte le varibili allocate con MALLOC e SHARED MEMORY
    */
    
    
    endSimulation(shmPidPorti, shmPidNavi, pid_meteo);

    while(wait(NULL) != -1);
    
    finalReport(statusNavi, statusMerci, maxOfferte, maxRichieste, merci_consegnate, statusPorti, portiSwell);
    
    shmdt(tipi_merci);
    shmdt(arr_richieste);
    shmdt(arr_offerte);
    shmdt(arr_pos_porti);
    shmdt(porti_selezionati);
    shmdt(arr_richieste_global);
    shmdt(arr_offerte_global);
    shmdt(merci_consegnate);
    shmdt(statusNavi);
    shmdt(statusMerci);
    shmdt(maxOfferte);
    shmdt(maxRichieste);
    shmdt(shmPidNavi);
    shmdt(shmPidPorti);
    shmdt(statusPorti);
    shmdt(portiSwell);


    shmctl(shm_merci_id, IPC_RMID, NULL);
    shmctl(shm_richieste_id, IPC_RMID, NULL);
    shmctl(shm_offerte_id, IPC_RMID, NULL);
    shmctl(shm_pos_porti_id, IPC_RMID, NULL);
    shmctl(shm_porti_selezionati_id, IPC_RMID, NULL);
    shmctl(shm_richieste_global_id, IPC_RMID, NULL);
    shmctl(shm_offerte_global_id, IPC_RMID, NULL);
    shmctl(shm_merci_cosegnate_id, IPC_RMID, NULL);
    shmctl(shm_statusNavi_id, IPC_RMID, NULL);
    shmctl(shm_statusMerci_id, IPC_RMID, NULL);
    shmctl(shm_maxOfferte_id, IPC_RMID, NULL);
    shmctl(shm_maxRichieste_id, IPC_RMID, NULL);
    shmctl(shm_pidNavi_id, IPC_RMID, NULL);
    shmctl(shm_pidPorti_id, IPC_RMID, NULL);
    shmctl(shm_statusPorti_id, IPC_RMID, NULL);
    shmctl(shm_portiSwell_id, IPC_RMID, NULL);

    free(pidNavi);
    free(pidPorti);

    printf("[SISTEMA] -> SIMULAZIONE TERMINATA\n");

    sigemptyset(&mask_block);
    sigaddset(&mask_block, SIGINT);
    sigprocmask(SIG_UNBLOCK, &mask_block, NULL);
}

/**
 * Generazione matrice offerte, tutte le offerte vengono generate in una matrice di 0 e 1 la quale e' la copia della matrice delle richieste ma invertita
 * Infine si effettua un controllo sull'ultimo porto, in modo che tutte le Merce siano offerte almeno una volta
*/

void gen_offerta(int matr_richieste[SO_PORTI][SO_MERCI+1], int matr_offerte[SO_PORTI][SO_MERCI+1], int * pidPorti, int num_merci, int print){
    int arr_control[SO_MERCI]= {0};
    int i, k, j, flag = 0, sum = 0, col_merce_offerte;
    int rand_pid, counter = SO_PORTI;

    for(i = 0; i < SO_PORTI; i++){
        matr_offerte[i][0] = pidPorti[i];
        for(j = 1; j < SO_MERCI + 1; j++){
            matr_offerte[i][j] = !matr_richieste[i][j];
        }
        j = 1;
        counter = 0;
    }
}
    
/**
 * Creazione matrice richieste, una matrice riempita di 0 e 1, la prima colonna viene riempita dai PID dei porti che richiederanno la merce. 
 * 0 e 1 servono per indicare se la corrispettiva merce indicata nella colonna e' richiesta o meno
 * Ogni porto richiede un numero casuale di Merce totali tra 1 e SO_MERCI/2
 * L'ultimo porto richiederà tutte le Merce che non sono state richieste dai restanti SO_PORTI - 1 porti, assicurandoci che tutte le Merce siano richieste
 * da almeno un porto
*/

void gen_richiesta_offerta(int * pidPorti, int * arr_richieste, int * arr_offerte, int print){
    int col_merce_richiesta, arr_control[SO_MERCI] = {0}, counter = 0;
    int matr_richieste[SO_PORTI][SO_MERCI+1] = {0}, matr_offerte[SO_PORTI][SO_MERCI + 1] = {0};
    int i, j, k, z;
    int sum = 0, flag = 0, num_richieste, num_merci;

    if(SO_MERCI == 1){
        num_merci = 1;
    }
    else{
        num_merci = SO_MERCI/2;
    }

    /**
     * Generazione delle Merce richeste dai primi SO_PORTI-1 porti, i controlli effettuati sulla generazione sono: 
     * 1) che il numero di richieste totali per singola merce non può eccedere SO_MERCI/2 (num_merci)
     * 2) che la merce non sia già stata richiesta dal porto corrente
     * In questo caso andiamo a gestire anche il caso in cui il numero di Merce totali e' 1, evitanto possibili crush o loop
    */

    if(SO_MERCI == 1){
        for(i = 0; i < SO_PORTI; i++){
            matr_richieste[i][0] = pidPorti[i];
            if(i < SO_PORTI/2){
                matr_richieste[i][1] = 1;
            }
            
        }
    }
    else{           
        for(j = 0; j < SO_PORTI-1; j++){
            matr_richieste[j][0] = pidPorti[j];
            num_richieste = (rand() % (num_merci)) + 1;
            for(k = 0; k < num_richieste; k++){
                col_merce_richiesta = (rand() % SO_MERCI)+1;
                for(i = 0; i < SO_PORTI-1; i++){
                    sum += matr_richieste[i][col_merce_richiesta];
                }
                while(matr_richieste[j][col_merce_richiesta] == 1 || sum >= SO_PORTI/2){
                    sum = 0;
                    col_merce_richiesta = (rand() % SO_MERCI) + 1;
                    for(i = 0; i < SO_PORTI-1; i++){
                        sum += matr_richieste[i][col_merce_richiesta];
                    }
                }
                matr_richieste[j][col_merce_richiesta] = 1;
            }
        }

        /**
         * Controllo sull'intera matrice, arr_control[i] = 0 se merce i non è mai stata richiesta dai primi SO_PORTI-1 porti
         * Tramite il contenuto di quest'ultimo array andremo ad assegnare all'ultimo porto tutte le risorse che non sono state richieste dagli altri
        */
        
        for(j = 0; j < SO_PORTI-1; j++){
            for(k = 1; k < SO_MERCI +1; k++){
                arr_control[k-1] |= matr_richieste[j][k];
            }
        }
        matr_richieste[SO_PORTI-1][0] = pidPorti[SO_PORTI-1];
        
        for(k = 1; k < SO_MERCI+1; k++){
            if(arr_control[k-1] == 0){
                matr_richieste[SO_PORTI-1][k] = 1;
                counter += 1;
            }
        }

        /**
         * Assegnazione delle rimanenti richieste per l'ultimo porto, escludendo quelle con troppe richieste (> SO_PORTI/2)
         * oppure già richieste dallo stesso porto (matr_richieste[SO_PORTI-1][col_merce_richiesta] == 1)
         * Quindi ogni merce in totale puà esser richiesta al masismo SO_PORTI/2 volte gestito tramite la variabile sum
        */

        flag = counter;
        for(k = 1; k <= SO_MERCI && flag < num_merci; k++){
            sum = 0;
            col_merce_richiesta = k;
            for(i = 0; i < SO_PORTI-1; i++){
                sum += matr_richieste[i][col_merce_richiesta];
            }
            if(matr_richieste[SO_PORTI-1][col_merce_richiesta] == 1 || sum >= SO_PORTI/2){
                continue;
            }
            matr_richieste[SO_PORTI-1][col_merce_richiesta] = 1;
            flag++;
        }
    }  

    /**
     * Una volta generata la tabella della richieste vado a generare la tabella delle offerte
    */
    
    gen_offerta(matr_richieste, matr_offerte,pidPorti, num_merci, print);

    /**
     * Generate le due matrici 
    */

    i = 0;
    for(j = 0; j < SO_PORTI; j++){
        for(k = 0; k < SO_MERCI+1; k++){
            arr_richieste[i] = matr_richieste[j][k];
            arr_offerte[i] = matr_offerte[j][k];
            i++;
        }
    }

    /**
     * Semplice stampa per testing
    */

    if(print){
        printf("\tRICHIESTE\n\n");
        for(j = 0; j < SO_PORTI; j++){
            printf("Pid: %d ", matr_richieste[j][0]);
            for(k = 1; k < SO_MERCI + 1; k++){
                printf("merce: %d,\tpresa: %d\t", k, matr_richieste[j][k]);
            }
            printf("\n");
        }
        printf("\n");

        printf("\tOFFERTE\n\n");
        for(j = 0; j < SO_PORTI; j++){
            printf("Pid: %d ", matr_offerte[j][0]);
            for(k = 1; k < SO_MERCI + 1; k++){
                printf("merce: %d,\tpresa: %d\t", k, matr_offerte[j][k]);
            }
            printf("\n");
        }
    }
    printf("[SISTEMA]\t -> \t MODULI DOMANDA/OFFERTA GENERATI CORRETTAMENTE\n");
}


/**
 * Questa funzione serve per effettuare un semplice controllo iniziale sulla maggior parte dei parametri inseriti all'interno del file env_var.h
 * Se un controllo non passa, il programma non parte
*/

void check_inputs(){
    if(SO_PORTI < 4){
        printf("[SISTEMA]\t -> \t ERRORE: IL NUMERO DI PORTI INSERITO E' INSUFFICENTE, BISOGNA INSERIRNE ALMENO 4\n");
        exit(- 1);
    }
    else if(SO_NAVI < 1){
        printf("[SISTEMA]\t -> \t ERRORE: IL NUMERO DI NAVI INSERITO E' < 0, BISOGNA INSERIRNE ALMENO UN NUMERO > 0\n");
        exit(- 1);
    }
    else if(SO_MIN_VITA > SO_MAX_VITA || SO_MIN_VITA < 0 || SO_MAX_VITA < 0){
        if(SO_MIN_VITA < 0){
            printf("[SISTEMA] \t -> \t ERRORE: LA DATA DI SCADENZA MINIMA NON PUO' ESSERE UN NUMERO NEGATIVO\n");
            exit(- 1);
        }
        else if(SO_MAX_VITA < 0){
            printf("[SISTEMA] \t -> \t ERRORE: LA DATA DI SCADENZA MASSIMA NON PUO' ESSERE UN NUMERO NEGATIVO\n");
            exit(- 1);
        }
        else{
            printf("[SISTEMA]\t -> \t ERRORE: LA DATA DI SCADENZA MINIMA NON PUO' ESSRE MAGGIORE DELLA SCADENZA MASSIMA\n");
            exit(- 1);
        }
    }
    else if(SO_LATO < 100){
        printf("[SISTEMA]\t ->  \t ERRORE: IL LATO DELLA MAPPA DEVE ESSERE >= 100\n");
        exit(- 1);
    }
    else if(SO_SPEED < 50){
        printf("[SISTEMA]\t -> \t ERRORE: LA VELOCITA' DELLE NAVI DEVE ESSERE UN NUMERO >= 50\n");
        exit(- 1);
    }
    else if(SO_CAPACITY < 10){
        printf("[SISTEMA]\t -> \t ERRORE: LA CAPACITA' DELLE NAVI DEVE ESSERE UN NUMERO >= 10\n");
        exit(- 1);
    }
    else if(SO_BANCHINE < 1){
        printf("[SISTEMA]\t -> \t ERRORE: LE BANCHINE PER PORTO DEVE ESSERE UN NUMERO >= 1\n");
        exit(- 1);
    }
    else if(SO_FILL < 1000){
        printf("[SISTEMA]\t -> \t ERRORE: SO_FILL DEVE ESSERE UN NUMERO >= 1000\n");
        exit(- 1);
    }
    else if(SO_LOADSPEED < 50){
        printf("[SISTEMA]\t -> \t ERRORE: LA VELOCITA' DI SCARICO DEVE ESSERE UN NUMERO >= 50\n");
        exit(- 1);
    }
    else if(SO_DAYS < 5){
        printf("[SISTEMA]\t -> \t ERRORE: SO_DAYS DEVE ESSERE UN NUMERO >= 5\n");
        exit(- 1);
    }
}

void dailyPrint(int * statusNavi, int * statusMerci, int * statusPorti, int * portiSwell, int giorno){
    int carico = 0;
    int noCarico = 0;
    int operandoPorto = 0;
    int naviAbbattute = 0;
    int naviBloccate = 0;
    int i;

    for(i = 0; i < SO_NAVI; i++){
        noCarico += statusNavi[(i * 6) + 1];
        carico += statusNavi[(i * 6) + 2];
        operandoPorto += statusNavi[(i * 6) + 3];
        naviAbbattute += statusNavi[(i * 6) + 4];
        naviBloccate += statusNavi[(i * 6) + 5];
    }
    printf("----------------------------------------------------------------------------------------------\n");
    printf("GIORNO\t->\t[%d]\n", giorno);
    printf("\n");
    printf("NAVI IN VIAGGIO SENZA CARICO\t->\t[%d]\n", noCarico);
    printf("NAVI IN VIAGGIO CON CARICO\t->\t[%d]\n", carico);
    printf("NAVI OPERANDO PRESSO PORTI\t->\t[%d]\n", operandoPorto);
    printf("NAVI CHE SONO STATE AFFONDATE\t->\t[%d]\n", naviAbbattute);
    printf("NAVI BLOCCATE DA STORM\t->\t[%d]\n", naviBloccate);
    printf("\n");
    printf("PORTI BLOCCATI DA SWELL\n");
    for(i = 0; i < giorno; i++){
        printf("[PORTO\t%d]\n", portiSwell[i]);
    }
    printf("\n");
    for(i = 0; i < SO_MERCI; i++){
        printf("Merce->%d presente in porto->%d\tpresente in nave->%d\tconsegnata in porto->%d\tscaduta in porto->%d\tscaduta in nave->%d\n", (i + 1), statusMerci[(i*5)], statusMerci[(i*5)+1], statusMerci[(i*5)+2], statusMerci[(i*5)+3], statusMerci[(i*5)+4]);
    }
    printf("\n");
    for(i = 0; i < SO_PORTI; i++){
        printf("[PORTO -> %d] Merci:\t presenti->%d\tricevute->%d\tspedite->%d\tbanchine->\tlibere %d su %d totali\n", statusPorti[(i * 6)], statusPorti[(i * 6) + 1], statusPorti[(i * 6) + 2], statusPorti[(i * 6) + 3], statusPorti[(i * 6) + 4], statusPorti[(i * 6) + 5]);
    }
    printf("----------------------------------------------------------------------------------------------\n");
    
}

void finalReport(int * statusNavi, int * statusMerci, int * maxOfferte, int * maxRichieste, int * merci_consegnate, int * statusPorti, int * portiSwell){
    int carico = 0;
    int noCarico = 0;
    int operandoPorto = 0;
    int naviAbbattute = 0;
    int naviBloccate = 0;
    int merciTot = 0;
    int i;

    printf("----------------------------------------------------------------------------------------------\n\n[SISTEMA]\t->\tREPORT FINALE\n\n");
    if(!flagEndMaterials){
        printf("[SISTEMA] -> TUTTE LE RICHIESTE SODDISFATTE OPPURE L'OFFERTA E' PARI A 0\n");
    }
    if(flagEndMaelstrom){
        printf("[SISTEMA] -> TUTTE LE NAVI SONO STATE COLPITE DAL MAELSTROM\n");
    }
    for(i = 0; i < SO_NAVI; i++){
        noCarico += statusNavi[(i * 6) + 1];
        carico += statusNavi[(i * 6) + 2];
        operandoPorto += statusNavi[(i * 6) + 3];
        naviAbbattute += statusNavi[(i * 6) + 4];
        naviBloccate += statusNavi[(i * 6) + 5];
    }
    printf("NAVI IN VIAGGIO SENZA CARICO\t->\t[%d]\n", noCarico);
    printf("NAVI IN VIAGGIO CON CARICO\t->\t[%d]\n", carico);
    printf("NAVI OPERANDO PRESSO PORTI\t->\t[%d]\n", operandoPorto);
    printf("NAVI CHE SONO STATE AFFONDATE\t->\t[%d]\n", naviAbbattute);
    printf("NAVI BLOCCATE DA STORM\t->\t[%d]\n", naviBloccate);
    printf("\n");
    printf("PORTI BLOCCATI DA SWELL\n");
    for(i = 0; i < SO_DAYS; i++){
        printf("[PORTO\t%d]\n", portiSwell[i]);
    }
    printf("\n");
    for(i = 0; i < SO_MERCI; i++){
        merciTot = statusMerci[(i * 5)] + statusMerci[(i * 5) + 1] + statusMerci[(i * 5) + 2] +  statusMerci[(i * 5) + 3] + statusMerci[(i * 5) + 4];
        printf("Merce -> %d: totale generata %d, di cui: presente in porto->%d\tpresente in nave->%d\tconsegnata in porto->%d\tscaduta in porto->%d scaduta in nave->%d\n", (i + 1),merciTot, statusMerci[(i*5)], statusMerci[(i*5)+1], merci_consegnate[i], statusMerci[(i*5)+3], statusMerci[(i*5)+4]);
        printf("Porto che ne ha offerta di più -> %d, Porto che ne ha richiesta di più -> %d\n\n", maxOfferte[(i*2)], maxRichieste[(i*2)]);
    }
    printf("\n");
    for(i = 0; i < SO_PORTI; i++){
        printf("[PORTO -> %d] Merci:\t presenti->%d\tricevute->%d\tspedite->%d\tbanchine->\tlibere %d su %d totali\n", statusPorti[(i * 6)], statusPorti[(i * 6) + 1], statusPorti[(i * 6) + 2], statusPorti[(i * 6) + 3], statusPorti[(i * 6) + 4], statusPorti[(i * 6) + 5]);
    }
    printf("----------------------------------------------------------------------------------------------\n");

    
}

void endSimulation (pid_t * pidPorti, pid_t * pidNavi, pid_t pidMeteo){
    int i;
    if(pidNavi != NULL){
        for (i = 0; i < SO_NAVI; i++){
            if(pidNavi[i] != -1){
                kill(pidNavi[i], SIGABRT);
            }
        }
    }
    for (i = 0; i < SO_PORTI; i++){
        kill(pidPorti[i], SIGABRT);
    }
    kill(pidMeteo, SIGABRT);
}