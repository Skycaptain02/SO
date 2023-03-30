#include "env_var.h"
#include "ipc.h"

int create_navi = 0;
int create_porti = 0;

void handler_navi_porti(int signal){
    switch(signal){
        case SIGUSR1:
            create_navi++;
            printf("Ho incrementato il valore\n");
        break;
        case SIGUSR2:
            create_porti++;
        break;
    }
}

int main(int argc, char * argv[]){
    
    struct sigaction sa;

    int i, status, errno, shm_merci_id, sem_config_id, j;
    pid_t * pid_navi, * pid_porti, pid_meteo;
    struct merci * tipi_merci;
    char buf[50];
    char* args_navi[] = {"./navi", "     ", NULL};
    char* args_porti[] = {"./porti", "     ", NULL};
    char* args_meteo[] = {"./meteo", "     ", NULL};
    char* args_merci[] = {"./merci", "       ", NULL};
    pid_navi = malloc(sizeof(pid_navi) * SO_NAVI);
    pid_porti = malloc(sizeof(pid_porti) * SO_PORTI);
    

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_navi_porti;
    sigaction(SIGUSR1, &sa, NULL);

    /*Sezione creazione shared memory per merci*/
    shm_merci_id = shmget(IPC_PRIVATE, sizeof(tipi_merci)*SO_MERCI, 0600 | IPC_CREAT);
    tipi_merci = shmat(shm_merci_id, NULL, 0);
    sprintf(buf, "%d", shm_merci_id);
    args_merci[1] = buf;
    /*Fine Sezione*/

    /*Sezione creazione semaforo per configurazione*/
    sem_config_id = semget(IPC_PRIVATE, 1, 0600 | IPC_CREAT);
    sem_set_val(sem_config_id, 0, (SO_NAVI+SO_PORTI+1));
    printf("Accessi: %d", semctl(sem_config_id, 0, GETVAL));
    sprintf(buf, "%d", sem_config_id);
    args_navi[1] = buf;
    args_porti[1] = buf;
    args_meteo[1] = buf;
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
        printf("Ritornato figlio merci\n");
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
            break;
            default:
            break;
        }
    }

    for(i = 0; i < SO_PORTI; i++){
        switch (pid_porti[i] = fork())
        {
            case -1:
                    printf("C'è stato un errore nel fork per i porti: %s", strerror(errno));
                    exit(-1);
                break;
            case 0:
                execve("./navi", args_navi , NULL);
            default:
            break;
        }

    }

    switch(pid_meteo = fork()){
            case -1:
                printf("C'è stato un errore nel fork per il meteo: %s", strerror(errno));
                exit(-1);
                break;
            case 0:
                execve("./meteo", args_meteo , NULL);
            default:
            break;
    }

   while(j = semctl(sem_config_id, 0, GETVAL) != 0);


    shmdt (tipi_merci);
    shmctl(shm_merci_id , IPC_RMID , NULL );
}
