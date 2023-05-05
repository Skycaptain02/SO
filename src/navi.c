#include "env_var.h"
#include "../lib/ipc.h"
#include "../lib/list.h"
#include <math.h>

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
    
    double ship_pos_x;
    double ship_pos_y;
    int sem_config_id, sem_offerte_richieste_id, msg_porti_navi_id;
    double * pos_porti;
    int shm_pos_porti_id, harbor_des, msg_bytes;
    struct sigaction sa;
    double distanza = 0, dist_parz_x, dist_parz_y, time_to_wait = 0;

    struct sigaction signal_blocker;
    sigset_t mask_block, mask_unblock;
    struct timespec tim, tim2;
    struct msgnotifica msg_notifica, msg_length;
    struct msgscarico msg_scarico;
    

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

    printf("[NAVE] -> HARBOR-PID: %f, HARBOR-X: %f, HARBOR-Y: %f\n", pos_porti[harbor_des * 3], pos_porti[harbor_des * 3 + 1], pos_porti[harbor_des * 3 + 2]);

    ship_pos_x = (rand() % (SO_LATO + 1)) - (SO_LATO / 2);
    ship_pos_y = (rand() % (SO_LATO + 1)) - (SO_LATO / 2);

    dist_parz_x = pow((pos_porti[harbor_des * 3 + 1]-ship_pos_x),2.0);

    dist_parz_y = pow((pos_porti[harbor_des * 3 + 2]-ship_pos_y),2);

    distanza = sqrt(dist_parz_x+dist_parz_y);

    printf("distanza tra nave e porto -> %f\n", distanza);

    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);
    sem_reserve(sem_config_id, 0);

    msg_porti_navi_id = msgget(getppid() , 0600 | IPC_CREAT);

    while(!flag_end){
        if(flag_day){
            sigemptyset (&mask_block );
            sigaddset (&mask_block , SIGUSR1) ;
            sigaddset (&mask_block , SIGABRT) ;
            sigprocmask(SIG_BLOCK, &mask_block, NULL);
            
            flag_day = 0;
            time_to_wait = distanza / SO_SPEED;
            printf("time to wait: %f\n", time_to_wait);
            tim.tv_sec = time_to_wait;
            nanosleep(&tim, &tim2);

            msg_notifica.type = pos_porti[harbor_des * 3];
            msg_notifica.pid = getpid();

            msgsnd(msg_porti_navi_id, &msg_notifica, sizeof(int), 0);

            msg_bytes = msgrcv(msg_porti_navi_id, &msg_length, sizeof(int) * 2, getpid(), 0);

            if(msg_bytes >= 0){

                msgsnd(msg_porti_navi_id, &msg_notifica, sizeof(int), 0);
                printf("lunghezza %d\n", msg_length.length);
                

                msg_bytes = msgrcv(msg_porti_navi_id, &msg_scarico, sizeof(merci) * msg_length.length, getpid(), 0);
                
                if(msg_bytes >= 0){
                    printf("HO RICEVUTO %d\n", msg_bytes);
                    printf("type -> %d, life -> %d\n", msg_scarico.merci[1].type, msg_scarico.merci[0].life);
                }
            }

            

            if (errno == EINTR) {
                continue;
            }
            else if(errno){
                printf("ERROR %s\n", strerror(errno));
            }
            
            sigemptyset (&mask_unblock );
            sigaddset (&mask_unblock , SIGUSR1) ;
            sigaddset (&mask_unblock , SIGABRT) ;
            sigprocmask(SIG_UNBLOCK, &mask_unblock, NULL);
            
        }

    }






}