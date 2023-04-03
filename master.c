#include "env_var.h"
#include "ipc.h"

void handler(int signal){
    
}

int main(int argc, char * argv[]){
    
    struct sigaction sa;

    int i, status, errno, shm_merci_id, shm_gen_id, sem_config_id; 
    int sem_porto_1,sem_porto_2,sem_porto_3,sem_porto_4; /*semafori per generare i primi 4 porti, uno per lato*/

    pid_t * pid_navi, * pid_porti, pid_meteo;
    struct merci * tipi_merci;
    char buf_idsem[50], buf_4harbour[50], buf_shm_merci[50];
    char* args_navi[] = {"./navi", NULL};
    char* args_porti[] = {"./porti", NULL};
    char* args_meteo[] = {"./meteo", NULL};
    char* args_merci[] = {"./merci", NULL};
    pid_navi = malloc(sizeof(pid_navi) * SO_NAVI);
    pid_porti = malloc(sizeof(pid_porti) * SO_PORTI);
    

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
                    printf("C'è stato un errore nel     fork per i porti: %s", strerror(errno));
                    exit(-1);
                break;
            case 0:
                sprintf(buf_4harbour, "%d", (i+1));
                args_porti[2] = buf_4harbour;
                execve("./porti", args_porti, NULL);
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
