#include "env_var.h"
#include "ipc.h"

void gen_offerta(int * arr_offerta, int  matr_richieste[SO_PORTI][SO_MERCI+1], int matr_offerte[SO_PORTI][SO_MERCI+1], int * pid_porti, int print);
void gen_richiesta(int  matr_richieste[SO_PORTI][SO_MERCI+1], int * pid_porti, int print);

void handler(int signal){
    
}

int main(int argc, char * argv[]){


    struct sigaction sa;

    int i, j, k, status, errno, shm_merci_id, shm_gen_id, sem_config_id;
    int sem_porto_1,sem_porto_2,sem_porto_3,sem_porto_4; /*semafori per generare i primi 4 porti, uno per lato*/
    int ric_1, ric_2, rand_pid;

    pid_t * pid_navi, * pid_porti, pid_meteo;
    struct merci * tipi_merci;
    char buf_idsem[50], buf_4harbour[50], buf_shm_merci[50];
    int matr_richieste[SO_PORTI][SO_MERCI+1] = {0}, matr_offerte[SO_PORTI][SO_MERCI+1] = {0}, arr_offerte[SO_PORTI], flag = 1;

    char* args_navi[] = {"./navi", NULL};
    char* args_porti[] = {"./porti", NULL};
    char* args_meteo[] = {"./meteo", NULL};
    char* args_merci[] = {"./merci", NULL};
    pid_navi = malloc(sizeof(pid_navi) * SO_NAVI);
    pid_porti = malloc(sizeof(pid_porti) * SO_PORTI);


    srand(getpid());
    

    /*bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_navi_porti;
    sigaction(SIGUSR1, &sa, NULL);*/

    /*Sezione creazione shared memory per merci*/
    shm_merci_id = shmget(getpid()+1, sizeof(tipi_merci)*SO_MERCI, 0600 | IPC_CREAT);
    tipi_merci = shmat(shm_merci_id, NULL, 0);
    sprintf(buf_shm_merci, "%d", shm_merci_id);
    args_merci[1] = buf_shm_merci;
    /*Fine Sezione*/

    /*Sezione creazione semaforo per configurazione*/
    sem_config_id = semget(getpid(), 1, 0600 | IPC_CREAT);
    sem_set_val(sem_config_id, 0, (SO_NAVI+SO_PORTI+1));
    sprintf(buf_idsem, "%d", sem_config_id);
    args_navi[1] = buf_idsem;
    args_porti[1] = buf_idsem;
    args_meteo[1] = buf_idsem;
    /*Fine Sezione*/

    switch(fork()){
        case 0:
                execve("./merci", args_merci , NULL);
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


    for(i = 0; i < SO_NAVI; i++){
        switch (pid_navi[i] = fork())
        {
            case -1:
                    printf("C'è stato un errore nel fork per le navi: %s", strerror(errno));
                    exit(-1);
                break;
            case 0:
                execve("./navi", args_navi , NULL);
                exit(0);
               
            break;
            default:
            break;
        }
    }


    
    /*Vado a creare i primi 4 porti su ogni lato della mappa*/
    for(i = 0; i < 4; i++){
        switch (pid_porti[i] = fork())
        {
            case -1:
                    printf("C'è stato un errore nel fork per i porti: %s", strerror(errno));
                    exit(-1);
                break;
            case 0:
                sprintf(buf_4harbour, "%d", (i+1));
                args_porti[2] = buf_4harbour;
                execve("./porti", args_porti, NULL);
                exit(0);
            default:    
            break;
        }
    }
    
    args_porti[2] = " ";
    
    /*Creo i restanti porti*/
    for(i = 4; i < SO_PORTI; i++){
        switch (pid_porti[i] = fork())
        {
            case -1:
                    printf("C'è stato un errore nel fork per i porti: %s", strerror(errno));
                    exit(-1);
                break;
            case 0:
                execve("./porti", args_porti , NULL);
                exit(0);
            default:
            break;
        }

    }
    
    gen_richiesta(matr_richieste, pid_porti, 1);
    gen_offerta(arr_offerte, matr_richieste, matr_offerte, pid_porti, 1);


    switch(pid_meteo = fork()){
            case -1:
                printf("C'è stato un errore nel fork per il meteo: %s", strerror(errno));
                exit(-1);
                break;
            case 0:
                execve("./meteo", args_meteo , NULL);
                exit(0);
            default:
            break;
    }

   while(semctl(sem_config_id, 0, GETVAL) != 0);
   
   for(i = 0; i < SO_PORTI; i++){
        kill(pid_porti[i], SIGUSR1);
   }

   /*Sezione creazione shared memory per allocare le merci ai porti*/
    shmget(getpid(), 4, 0600 | IPC_CREAT);
    /*Fine Sezione*/

    while(wait(NULL) != -1);

    shmdt (tipi_merci);
    shmctl(shm_merci_id , IPC_RMID , NULL);
}


/**
 * Creazione di un array di indici casuali, non doppioni arr_offerta
 * ogni riga della matrice delle offerte sarà associata una riga della matrice delle richieste, scelta tramite i valori dell'array precedente
 * la generazione di tali indici e' controllata in modo che ogni porto non potrà avere richiete = offerta
*/
void gen_offerta(int * arr_offerta, int  matr_richieste[SO_PORTI][SO_MERCI+1], int matr_offerte[SO_PORTI][SO_MERCI+1], int * pid_porti, int print){
    int arr_copy[SO_PORTI];
    int i, k, j, rand_pid, counter = SO_PORTI;

    for(i = 0; i < SO_PORTI; i++){
        arr_copy[i] = i;
        arr_offerta[i] = - 1;
    }
    i = 0;
    while(i != SO_PORTI){
        rand_pid = rand() % SO_PORTI;
        if(arr_offerta[i] == - 1 && arr_copy[rand_pid] != - 1 && rand_pid != i){
            arr_offerta[i] = arr_copy[rand_pid];
            arr_copy[rand_pid] = - 1;
            i++;
            counter--;
        }
        if(counter == 1){
            for(k = 0; k < SO_PORTI; k++){
                if(arr_copy[k] != -1){
                    arr_offerta[i] = arr_copy[k];
                    arr_copy[rand_pid] = - 1;
                }
            }
            i++;
        }
    }
    
    for(i = 0; i < SO_PORTI; i++){
        matr_offerte[i][0] = pid_porti[i];
        for(k = 1; k < SO_MERCI + 1; k++){
            matr_offerte[i][k] = matr_richieste[arr_offerta[i]][k];
        }
    }

    if(print){
        printf("\tOFFERTE\n");
        for(j = 0; j < SO_PORTI; j++){
            printf("Pid: %d ", matr_offerte[j][0]);
            for(k = 1; k < SO_MERCI + 1; k++){
                printf("merce: %d,\tpresa: %d\t", k, matr_offerte[j][k]);
            }
            printf("\n");
        }
    }
}
    
/**
 * Creazione matrice richieste, una matrice riempita di 0 e 1, la prima colonna viene riempita dai PID dei porti che richiedono la merce. 0 e 1 servono per indicare se
 * la corrispettiva merce indicata nella colonna e' richiesta o meno
 * L'unico controllo effettuato e' che ogni porto richiede SO_MERCI/2 merci e nel caso in cui una o più merci specifiche non vengano richieste da alcun porto, l'ultimo porto 
 * richiederà tutte le merci non richieste dagli altri. 
 * Quest'ultimo porto e' gestito tramite un arr_control
*/
void gen_richiesta(int  matr_richieste[SO_PORTI][SO_MERCI+1], int * pid_porti, int print){
    int ric_1, arr_control[SO_MERCI] = {0}, counter = 0;
    int j, k;
    for(j = 0; j < SO_PORTI-1; j++){
        matr_richieste[j][0] = pid_porti[j];
        for(k = 0; k < SO_MERCI/2; k++){
            ric_1 = (rand() % SO_MERCI)+1;
            while(matr_richieste[j][ric_1] == 1){
                ric_1 = (rand() % SO_MERCI)+1;
            }
            matr_richieste[j][ric_1] = 1;
            
        }
    }
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
    for(k = 0; k < (SO_MERCI/2)-counter; k++){
        ric_1 = (rand() % SO_MERCI)+1;
        while(matr_richieste[SO_PORTI-1][ric_1] == 1){
            ric_1 = (rand() % SO_MERCI)+1;
        }
        matr_richieste[SO_PORTI-1][ric_1] = 1;
    }
    if(print){
        printf("\tRICHIESTE\n");
        for(j = 0; j < SO_PORTI; j++){
            printf("Pid: %d ", matr_richieste[j][0]);
            for(k = 1; k < SO_MERCI + 1; k++){
                printf("merce: %d,\tpresa: %d\t", k, matr_richieste[j][k]);
            }
            printf("\n");
        }
    }
 
}
