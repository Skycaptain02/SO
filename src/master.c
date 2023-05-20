#include "env_var.h"
#include "../lib/ipc.h"

void gen_richiesta_offerta(int * pid_porti, int * arr_richieste, int * arr_offerte, int print);
void gen_offerta(int matr_richieste[SO_PORTI][SO_MERCI+1], int matr_offerte[SO_PORTI][SO_MERCI+1], int * pid_porti, int num_merci, int print);
void check_inputs();

int main(int argc, char * argv[]){

    int shm_porti_selezionati_id, shm_pos_porti_id, shm_merci_id, shm_richieste_id, shm_offerte_id, shm_richieste_global_id, shm_offerte_global_id, shm_merci_cosegnate_id;
    int * arr_richieste, * arr_offerte, * arr_richieste_global, * arr_offerte_global;
    double * arr_pos_porti;
    int * porti_selezionati, * merci_consegnate;
    Merce * tipi_merci;

    sigset_t mask_block;


    struct sigaction sa;
    int i, j, k, z, status, errno;

    
    int sem_config_id = 0, sem_offerte_richieste_id = 0;
    int sem_porto_1,sem_porto_2,sem_porto_3,sem_porto_4; 
    int * porti_random, * random_index;

    pid_t * pid_navi, * pid_porti, pid_meteo;
   
    int porto_scelto; 
    int flag = 1;
    char argv_buffer[50];

    char * args_navi[]  = {"navi.c", NULL};
    char * args_porti[] = {"porti.c", NULL};
    char * args_meteo[] = {"meteo.c", NULL};
    char * args_merci[] = {"merci.c", NULL};
    
    pid_navi = malloc(sizeof(pid_navi) * SO_NAVI);
    pid_porti = malloc(sizeof(pid_porti) * SO_PORTI);

    sigemptyset(&mask_block);
    sigaddset(&mask_block, SIGINT);
    sigprocmask(SIG_BLOCK, &mask_block, NULL);

    srand(getpid());

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
    */

    shm_merci_id = shmget(getpid() + 1, sizeof(Merce) * SO_MERCI, 0600 | IPC_CREAT);
    tipi_merci = shmat(shm_merci_id, NULL, 0);
    
    shm_richieste_id = shmget(getpid() + 2, sizeof(int)* ((SO_MERCI + 1)*SO_PORTI), 0600 | IPC_CREAT);
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
        merci_consegnate[i] = 0;
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

    /**
     * Generazione dei primi 4 porti su ogni lato della mappa
    */

    for(i = 0; i < 4; i++){
        switch (pid_porti[i] = fork()){
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
        switch (pid_porti[i] = fork())
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
        arr_pos_porti[i * 3] = (unsigned int)pid_porti[i];
        arr_offerte_global[i * (SO_MERCI + 1)] = (unsigned int)pid_porti[i];
        arr_richieste_global[i * (SO_MERCI + 1)] = (unsigned int)pid_porti[i];
    }


    /**
     * Dopo aver generato tutti i processi porti, vado a creare le matrici delle richieste e delle offerte, creade un semaforo che comunicherà
     * ai porti quando iniziare a generare le effettive risorse all'interno del loro magazzino
    */

    printf("[SISTEMA]\t -> \t TUTTI I PORTI SONO PRONTI\n");

    gen_richiesta_offerta(pid_porti, arr_richieste, arr_offerte, 0);
    sem_reserve(sem_offerte_richieste_id, 0);
    
    /**
     * Generazionie di tutte le navi
    */
    
    for(i = 0; i < SO_NAVI; i++){
        switch (pid_navi[i] = fork()){
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

    printf("[SISTEMA]\t -> \t TUTTE LE NAVI SONO PRONTE\n");

    /**
     * Generazione processo meteo
    */
 
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
     * Dopo aver creato il processo meteo, il master aspetta che tutti i propri processi figli terminino la loro configurazione iniziale
     * Subito dopo verrà inviato un segnale SIGUSR1 in modo che i porti inizieranno a generare le risorse facendo cominciare la simulazione
    */

    while(semctl(sem_config_id, 0, GETVAL) != 0);

    printf("[SISTEMA]\t -> \t INIZIO SIMULAZIONE\n");

    /**
     * La simulazione effettiva comincia qui
     * All'inizio del ciclo while vengono scelti un numero casuale si porti i quali dovranno generare risorse all'interno del loro magazzino
     * Tali porti verranno scelti a caso, non ripetuti, in modo che non si verichi un possibile merge di segnali
     * I porti che giorno N sono stati scelti riceveranno dal master un segnale SIGUSR2 il quale comunicherà ai porti di creare le proprie Merce
    */

    i = 0;
    k = 0;
    while(i != SO_DAYS){
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
        printf("GIORNO -> [%d]\n", (i+1));
        i++;
        for(j = 0; j < * porti_selezionati; j++){
            kill(pid_porti[porti_random[j]], SIGUSR2);
        }

        j = 0;
        k = 0;

        sleep(1);
        free(porti_random);
    }

    for(i = 0; i < SO_MERCI; i++){
        printf("Merce %d, consegnata %d volte\n", i+1, merci_consegnate[i]);
    }
    
    /**
     * Fine sumulazione, invio un SIGABRT a tutti i processi indicandogli di terminare, subito dopo dealloco tutte le varibili allocate con MALLOC e SHARED MEMORY
    */

    
    
    for (i = 0; i < SO_NAVI; i++){
        kill(pid_navi[i], SIGABRT);
    }
    for (i = 0; i < SO_PORTI; i++){
        kill(pid_porti[i], SIGABRT);
    }

    
    

    while(wait(NULL) != -1);
    
    
    

    shmdt(porti_selezionati);
    shmdt(arr_pos_porti);
    shmdt(tipi_merci);
    shmdt(arr_richieste);
    shmdt(arr_offerte);
    shmdt(arr_richieste_global);
    shmdt(arr_offerte_global);
    shmdt(merci_consegnate);

    shmctl(shm_porti_selezionati_id, IPC_RMID, NULL);
    shmctl(shm_pos_porti_id, IPC_RMID, NULL);
    shmctl(shm_merci_id, IPC_RMID, NULL);
    shmctl(shm_richieste_id, IPC_RMID, NULL);
    shmctl(shm_offerte_id, IPC_RMID, NULL);
    shmctl(shm_richieste_global_id, IPC_RMID, NULL);
    shmctl(shm_offerte_global_id, IPC_RMID, NULL);
    shmctl(shm_merci_cosegnate_id, IPC_RMID, NULL);

    free(pid_navi);
    free(pid_porti);

    printf("[SISTEMA] -> SIMULAZIONE TERMINATA\n");

    sigemptyset(&mask_block);
    sigaddset(&mask_block, SIGINT);
    sigprocmask(SIG_UNBLOCK, &mask_block, NULL);
}

/**
 * Generazione matrice offerte, tutte le offerte vengono generate in una matrice di 0 e 1 la quale e' la copia della matrice delle richieste ma invertita
 * Infine si effettua un controllo sull'ultimo porto, in modo che tutte le Merce siano offerte almeno una volta
*/

void gen_offerta(int matr_richieste[SO_PORTI][SO_MERCI+1], int matr_offerte[SO_PORTI][SO_MERCI+1], int * pid_porti, int num_merci, int print){
    int arr_control[SO_MERCI]= {0};
    int i, k, j, flag = 0, sum = 0, col_merce_offerte;
    int rand_pid, counter = SO_PORTI;

    for(i = 0; i < SO_PORTI; i++){
        matr_offerte[i][0] = pid_porti[i];
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

void gen_richiesta_offerta(int * pid_porti, int * arr_richieste, int * arr_offerte, int print){
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
            matr_richieste[i][0] = pid_porti[i];
            if(i < SO_PORTI/2){
                matr_richieste[i][1] = 1;
            }
            
        }
    }
    else{           
        for(j = 0; j < SO_PORTI-1; j++){
            matr_richieste[j][0] = pid_porti[j];
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
        matr_richieste[SO_PORTI-1][0] = pid_porti[SO_PORTI-1];
        
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
    
    gen_offerta(matr_richieste, matr_offerte,pid_porti, num_merci, print);

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
    else if(SO_NAVI < 0){
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
    else if(SO_LATO <= 1){
        printf("[SISTEMA]\t ->  \t ERRORE: IL LATO DELLA MAPPA DEVE ESSERE > 1\n");
        exit(- 1);
    }
    else if(SO_SPEED <= 0){
        printf("[SISTEMA]\t -> \t ERRORE: LA VELOCITA' DELLE NAVI DEVE ESSERE UN NUMERO > 0\n");
        exit(- 1);
    }
    else if(SO_CAPACITY <= 0){
        printf("[SISTEMA]\t -> \t ERRORE: LA CAPACITA' DELLE NAVI DEVE ESSERE UN NUMERO > 0\n");
        exit(- 1);
    }
    else if(SO_BANCHINE <= 0){
        printf("[SISTEMA]\t -> \t ERRORE: LE BANCHINE PER PORTO DEVE ESSERE UN NUMERO > 0\n");
        exit(- 1);
    }
    else if(SO_FILL <= 0){
        printf("[SISTEMA]\t -> \t ERRORE: SO_FILL DEVE ESSERE UN NUMERO > 0\n");
        exit(- 1);
    }
    else if(SO_LOADSPEED <= 0){
        printf("[SISTEMA]\t -> \t ERRORE: LA VELOCITA' DI SCARICO DEVE ESSERE UN NUMERO > 0\n");
        exit(- 1);
    }
    else if(SO_DAYS <= 0){
        printf("[SISTEMA]\t -> \t ERRORE: SO_DAYS DEVE ESSERE UN NUMERO > 0\n");
        exit(- 1);
    }
}
