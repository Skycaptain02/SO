#include "env_var.h"
#include "../lib/ipc.h"
#include "../lib/list.h"
#include <math.h>

#define CONVERSION_SEC_NSEN 1000000000

void travel(double distanza);

int flag_end = 0, flag_day = 0;

void handler_start(int signal){
    switch(signal){
        case SIGUSR1:
            flag_day = 1;
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
    
    double ship_pos_x, dist_parz_x;
    double ship_pos_y, dist_parz_y;
    int harbor_des;
    int sem_config_id, sem_offerte_richieste_id;

    int msg_porti_navi_id;
    int msg_bytes;
    double distanza;

    int shm_pos_porti_id, shm_richieste_local_id, shm_offerte_local_id;
    int * richieste_local, * offerte_local, * porti_selezionati;
    double * pos_porti;
    struct sigaction sa;
    struct msgOp Operation;
    
    srand(getpid());

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_start;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR1, &sa, NULL);

    sem_offerte_richieste_id = semget(getppid() + 1, 1, 0600 | IPC_CREAT);
    while(semctl(sem_offerte_richieste_id, 0, GETVAL) != 0);

    shm_pos_porti_id = shmget(getppid() + 4, sizeof(double) * (SO_PORTI * 3), 0600 | IPC_CREAT);
    pos_porti = shmat(shm_pos_porti_id, NULL, 0);  

    harbor_des = rand() % SO_PORTI;

    ship_pos_x = (rand() % (SO_LATO + 1)) - (SO_LATO / 2);
    ship_pos_y = (rand() % (SO_LATO + 1)) - (SO_LATO / 2);

    dist_parz_x = pow((pos_porti[harbor_des * 3 + 1] - ship_pos_x), 2);
    dist_parz_y = pow((pos_porti[harbor_des * 3 + 2] - ship_pos_y), 2);
    distanza = sqrt(dist_parz_x+dist_parz_y);

    printf("distanza tra nave e porto -> %f\n", distanza);

    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);
    sem_reserve(sem_config_id, 0);

    msg_porti_navi_id = msgget(getppid() , 0600 | IPC_CREAT);

    while(!flag_end){
        travel(distanza);
        Operation.type = pos_porti[harbor_des * 3];
        Operation.operation = 0;
        msgsnd(msg_porti_navi_id, &Operation, sizeof(int),0);

        msg_bytes = msgrcv(msg_porti_navi_id, &Operation, sizeof(int), pos_porti[harbor_des * 3], 0);

        if(msg_bytes >= 0){
            if(Operation.operation = 0){
                printf("Posso attraccare\n");
            }
            else{
                printf("Me ne vado\n");
            }
            
        }
    }
}


void travel(double distanza){
    struct timespec tim, tim2;
    sigset_t mask_block, mask_unblock;
    sigemptyset (&mask_block);
    sigaddset (&mask_block , SIGUSR1);
    sigaddset (&mask_block , SIGABRT);
    sigprocmask(SIG_BLOCK, &mask_block, NULL);
    tim.tv_sec = (distanza / SO_SPEED) ;
    printf("DEVO ASPETTARE -> %ld SECONDI \n", tim.tv_sec);
    nanosleep(&tim, &tim2);
    sigemptyset (&mask_unblock );
    sigaddset (&mask_unblock , SIGUSR1) ;
    sigaddset (&mask_unblock , SIGABRT) ;
    sigprocmask(SIG_UNBLOCK, &mask_unblock, NULL);
}