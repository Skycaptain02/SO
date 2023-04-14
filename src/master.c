#include "env_var.h"
#include "../lib/ipc.h"

void gen_richiesta_offerta(int * pid_porti, int * arr_richieste, int * arr_offerte, int print);
void gen_offerta(int matr_richieste[SO_PORTI][SO_MERCI+1], int matr_offerte[SO_PORTI][SO_MERCI+1], int * pid_porti, int print);

void handler(int signal){
    
}

int main(int argc, char * argv[]){

    struct sigaction sa;

    int i, j, k, status, errno;
    int shm_pos_navi_id, shm_pos_porti_id, shm_merci_id, shm_richieste_id, shm_offerte_id, sem_config_id;
    int sem_porto_1,sem_porto_2,sem_porto_3,sem_porto_4; /*semafori per generare i primi 4 porti, uno per lato*/
    int ric_1, ric_2, rand_pid;
    int * arr_richieste, * arr_offerte;
    double * arr_pos_porti, * arr_pos_navi;

    pid_t * pid_navi, * pid_porti, pid_meteo;
    struct merci * tipi_merci;
    int flag = 1;
    char buf_4harbour[50];

    char* args_navi[] = {"./navi", NULL};
    char* args_porti[] = {"./porti", "      ", NULL};
    char* args_meteo[] = {"./meteo", NULL};
    char* args_merci[] = {"./merci", NULL};
    
    pid_navi = malloc(sizeof(pid_navi) * SO_NAVI);
    pid_porti = malloc(sizeof(pid_porti) * SO_PORTI);
    arr_richieste = malloc(sizeof(int) * SO_PORTI * (SO_MERCI + 1));
    arr_offerte = malloc(sizeof(int) * SO_PORTI * (SO_MERCI + 1));
    arr_pos_porti = malloc(sizeof(double) * (SO_PORTI * 3));
    arr_pos_navi = malloc(sizeof(double) * (SO_PORTI * 3));

    srand(getpid());

    if(SO_PORTI < 4){
        printf("[SISTEMA]\t -> \t IL NUMERO DI PORTI INSERITO E' INSUFFICENTE, ALMENO 4\n");
        exit(-1);
    }
    

    /*bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_navi_porti;
    sigaction(SIGUSR1, &sa, NULL);*/

    /**
     * parent pid +0: shared memory fill
     * parent pid +1: shared memory merci
     * parent pid +2: shared memory richieste
     * parend pid +3: shared memory offerte
     * parent pid +4: shared memory posizione navi
     * parent pid +5: shared memory posizione porti
    */

    /* Sezione creazione shared memory per merci */
        shm_merci_id = shmget(getpid()+1, sizeof(tipi_merci)*SO_MERCI, 0600 | IPC_CREAT);
        tipi_merci = shmat(shm_merci_id, NULL, 0);
    /*Sezione creazione semaforo per configurazione*/

    /*Sezione creazione shared memory per offerte e richieste*/
        shm_richieste_id = shmget(getpid()+2, sizeof(int)* ((SO_MERCI+1)*SO_PORTI), 0600 | IPC_CREAT);
        arr_richieste = shmat(shm_richieste_id, NULL, 0);

        shm_offerte_id = shmget(getpid()+3, sizeof(int) * ((SO_MERCI + 1) * SO_PORTI), 0600 | IPC_CREAT);
        arr_offerte = shmat(shm_offerte_id, NULL, 0);
    /*fIne Sezione creazione shared memory per offerte e richieste*/

    /*Sezione creazione shared memory per posizione navi e porti*/
        /*shm_pos_navi_id = shmget(getpid() + 4, sizeof(double)* (SO_NAVI * 3), 0600 | IPC_CREAT);
        arr_pos_navi = shmat(shm_pos_navi_id, NULL, 0);*/

        shm_pos_porti_id = shmget(getpid() + 5, sizeof(double) * (SO_PORTI * 3), 0600 | IPC_CREAT);
        arr_pos_porti = shmat(shm_pos_porti_id, NULL, 0);
    /*fIne Sezione creazione shared memory per posizione navi e porti*/
   
    /*Sezione creazione semaforo per configurazione*/
        sem_config_id = semget(getpid(), 1, 0600 | IPC_CREAT);
        sem_set_val(sem_config_id, 0, (SO_NAVI + SO_PORTI + 1));
    /*Fine Sezione crezione semaforo per configuazione*/

    switch(fork()){
        case 0:
                execve("../bin/merci", args_merci , NULL);
                exit(0);
            break;
        default:
        break;
    }

    while(wait(NULL) != -1){
        /*printf("Ritornato figlio merci\n");*/
    }

    /*for(i = 0; i < SO_MERCI; i++){
        printf("Tipo: %d, Peso: %d, Vita: %d\n", tipi_merci[i].type, tipi_merci[i].weight, tipi_merci[i].life);
    }*/

    /**
     * Generazionie di tutte le navi
    */

    for(i = 0; i < SO_NAVI; i++){
        switch (pid_navi[i] = fork())
        {
            case -1:
                    printf("C'è stato un errore nel fork per le navi: %s", strerror(errno));
                    exit(-1);
                break;
            case 0:
                execve("../bin/navi", args_navi , NULL);
                exit(0);
               
            break;
            default:
                /*arr_pos_navi[i*SO_NAVI] = pid_navi[i];*/
            break;
        }
    }

    

    printf("[SISTEMA]\t -> \t TUTTE LE NAVI SONO PRONTE\n");

    /**
     * Generazione dei primi 4 porti su ogni lato della mappa
    */

    for(i = 0; i < 4; i++){
        switch (pid_porti[i] = fork())
        {
            case -1:
                    printf("C'è stato un errore nel fork per i porti: %s", strerror(errno));
                    exit(-1);
                break;
            case 0:
                sprintf(buf_4harbour, "%d", (i+1));
                args_porti[1] = buf_4harbour;
                execve("../bin/porti", args_porti, NULL);
                exit(0);
            default:
                arr_pos_porti[i * SO_PORTI] = pid_porti[i];    
            break;
        }
    }
    
    args_porti[1] = " ";
    
    /**
     * Creo i restanti porti
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
            default:
                arr_pos_porti[i * SO_PORTI] = pid_porti[i];
            break;
        }
    }

    gen_richiesta_offerta(pid_porti, arr_richieste, arr_offerte, 0);
    
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

    while(semctl(sem_config_id, 0, GETVAL) != 0){
    };

    


    for(i = 0; i < SO_PORTI; i++){
        kill(pid_porti[i], SIGUSR1);
    }

    for(i = 0; i < SO_NAVI; i++){
        kill(pid_navi[i], SIGUSR1);
    }

   printf("[SISTEMA]\t -> \t TUTTI I PORTI SONO PRONTI\n");

   /*Sezione creazione shared memory per fill*/
    shmget(getpid(), 4, 0600 | IPC_CREAT);
    /*Fine Sezione*/

    while(wait(NULL) != -1);

    shmdt (tipi_merci);
    shmctl(shm_merci_id , IPC_RMID , NULL);


    for(i = 0; i < SO_PORTI;i++){
        printf("PID -> %f, POSX -> %f, POSY -> %f\n", arr_pos_porti[i*SO_PORTI], arr_pos_porti[i*SO_PORTI+1], arr_pos_porti[i*SO_PORTI+2]);
    }
    

}

/**
 * Generazione matrice offerte, tutte le offerte vengono generate in una matrice di 0 e 1 la quale e' la copia della matrice delle richieste ma invertita
 * questo processo e' randomico, si sceglie casualemente se invertire o meno il contenuto nella matrice delle richieste
 * Infine si effettua un controllo sull'ultimo porto, in modo che tutte le merci siano offerte almeno una volta
*/

void gen_offerta(int matr_richieste[SO_PORTI][SO_MERCI+1], int matr_offerte[SO_PORTI][SO_MERCI+1], int * pid_porti, int print){
    int arr_control[SO_MERCI]= {0};
    int i, k, j, flag = 0, sum = 0, col_merce_offerte;
    int rand_pid, counter = SO_PORTI;

    for(i = 0; i < SO_PORTI-1; i++){
        matr_offerte[i][0] = pid_porti[i];
        for(j = 1; j < SO_MERCI; j++){
            rand_pid = rand() % 101;
            matr_offerte[i][j] = (matr_richieste[i][j] == 1) ?  0 : 1;
            if(rand_pid < 30){
                matr_offerte[i][j] = 0;
            }
        }
        j = 1;
        counter = 0;
    }

    /**
     * Controllo sull'intera matrice, arr_control[i] = 0 se merce i non è mai stata offerta dai primi SO_PORTI-1 porti
     * Tramite il contenuto di quest'ultimo array andremo ad assegnare all'ultimo porto tutte le risorse che non sono state offerte dagli altri
    */
    
    for(j = 0; j < SO_PORTI-1; j++){
        for(k = 1; k < SO_MERCI +1; k++){
            arr_control[k-1] |= matr_offerte[j][k];
        }
    }
    matr_offerte[SO_PORTI-1][0] = pid_porti[SO_PORTI-1];
    
    /**
     * Assegnazione all'SO_PORTI porto delle risorse non offerte da nessun'altro porto
    */
    
    for(k = 1; k < SO_MERCI+1; k++){
        if(arr_control[k-1] == 0){
            matr_offerte[SO_PORTI-1][k] = 1;
            counter += 1;
        }
    }

    /**
     * Assegnazione delle rimanenti offerte per l'ultimo porto, escludendo quelle con troppe richieste (> SO_PORTI/2)
     * oppure già offerte dallo stesso porto (matr_offerte[SO_PORTI-1][col_merce_offerta] == 1)
     * Quindi ogni merce in totale puà esser offerta al masismo SO_PORTI/2 volte gestito tramite la variabile sum
    */

    flag = counter;
    for(k = 1; k <= SO_MERCI && flag < SO_MERCI/2; k++){
        sum = 0;
        col_merce_offerte = k;
        for(i = 0; i < SO_PORTI-1; i++){
            sum += matr_offerte[i][col_merce_offerte];
        }
        if(matr_offerte[SO_PORTI-1][col_merce_offerte] == 1 || sum >= SO_PORTI/2){
            continue;
        }
        matr_offerte[SO_PORTI-1][col_merce_offerte] = 1;
        flag++;
    }
}
    
/**
 * Creazione matrice richieste, una matrice riempita di 0 e 1, la prima colonna viene riempita dai PID dei porti che richiedono la merce. 
 * 0 e 1 servono per indicare se la corrispettiva merce indicata nella colonna e' richiesta o meno
 * Ogni porto richiede un numero casuale di merci totali tra 1 e SO_MERCI/2
 * L'ultimo porto richiederà tutte le merci che non sono state richieste dai restanti SO_PORTI - 1 porti, assicurandoci che tutte le merci siano richieste
 * da almeno un porto
*/

void gen_richiesta_offerta(int * pid_porti, int * arr_richieste, int * arr_offerte, int print){
    int col_merce_richiesta, arr_control[SO_MERCI] = {0}, counter = 0;
    int matr_richieste[SO_PORTI][SO_MERCI+1] = {0}, matr_offerte[SO_PORTI][SO_MERCI + 1] = {0};
    int i, j, k, z;
    int sum = 0, flag = 0, num_richieste;
    
    /**
     * Generazione delle merci richeste dai primi SO_PORTI-1 porti, i controlli effettuati sulla generazione sono: 
     * 1) che il numero di richieste totali per singola merce non può eccedere SO_MERCI/2
     * 2) che la merce non sia già stata richiesta dal porto corrente
    */
           
    for(j = 0; j < SO_PORTI-1; j++){
        matr_richieste[j][0] = pid_porti[j];
        num_richieste = (rand() % (SO_MERCI/2)) + 1;
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
    
    /**
     * Assegnazione all'SO_PORTI porto delle risorse non richieste da nessun'altro porto
    */
    
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
    for(k = 1; k <= SO_MERCI && flag < SO_MERCI/2; k++){
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

    

    /**
     * Una volta generata la tabella della richieste vado a generare la tabella delle offerte
    */
    
    gen_offerta(matr_richieste, matr_offerte,pid_porti, print);
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
