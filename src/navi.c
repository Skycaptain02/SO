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
    int msg_bytes, riga_matrice = -1;
    double distanza;

    int shm_pos_porti_id, shm_richieste_local_id, shm_offerte_local_id;
    int * richieste_local, * offerte_local, * porti_selezionati;
    double * pos_porti;
    int current_weight = 0;
    struct sigaction sa;
    struct msgOp Operation, Info_vita;
    int * arr_richieste_global, * arr_offerte_global;
    int i, merce_rand;
    node * stiva;
    int shm_merci_id;
    merci * tipi_merce;
    merci temp;
    
    srand(getpid());

    shm_merci_id = shmget(getppid() + 1, sizeof(tipi_merce) * SO_MERCI, 0600 | IPC_CREAT);
    tipi_merce = shmat(shm_merci_id, NULL, 0);

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
    for(i = 0; i < SO_PORTI; i++){
        printf("PID -> %d, POS_X -> %f, POS_Y -> %f\n", (unsigned int)pos_porti[i*3], pos_porti[i*3+1], pos_porti[i*3+2]);
    }
    
    while(!flag_end){
        travel(distanza);
        Operation.type = (unsigned int)pos_porti[harbor_des * 3];
        Operation.operation = 0;
        msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2,0);                                                   
        msg_bytes = msgrcv(msg_porti_navi_id, &Operation, sizeof(int) * 2, (unsigned int)pos_porti[harbor_des * 3], 0);           
        printf("HO RICEVUTO DA QUESTO PORTO -> %d\n", (unsigned int)pos_porti[harbor_des * 3]);
        printf("Ho ricevuto op: %d, bytes %d\n", Operation.operation, msg_bytes);
        if(msg_bytes >= 0){
            if(Operation.operation == 0){                                                                           
                printf("Posso attraccare\n");
                Operation.operation = (current_weight > 0) ? 1 : 2;                                                  
                msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2, 0);                                           
                msg_bytes = msgrcv(msg_porti_navi_id, &Operation, sizeof(int) * 2, (unsigned int)pos_porti[harbor_des * 3], 0);   
                if(msg_bytes >= 0 && Operation.operation == 3){ 
                    printf("CIAOOO\n");                                                   
                    if(current_weight > 0){
                        shm_richieste_local_id = shmget(getppid() + 6, sizeof(int) * ((SO_MERCI +1 ) * SO_PORTI), 0600);
                        arr_richieste_global = shmat(shm_richieste_local_id, NULL, 0);
                        if(riga_matrice == -1){
                            i = 0;
                            while(arr_richieste_global[i*(SO_MERCI+1)] != (unsigned int)pos_porti[harbor_des * 3]){
                                i++;
                            }
                            riga_matrice = i;
                            i = 0;
                        }
                        

                        printf("SCARICO\n");
                    }
                    else{
                        printf("CARICO\n");
                        shm_offerte_local_id = shmget(getppid() + 6, sizeof(int) * ((SO_MERCI +1 ) * SO_PORTI), 0600);
                        arr_offerte_global = shmat(shm_offerte_local_id, NULL, 0);
                        if(riga_matrice == -1){
                            i = 0;
                            while(arr_offerte_global[i * (SO_MERCI+1)] != (unsigned int)pos_porti[harbor_des * 3]){
                                i++;
                            }
                            riga_matrice = i;
                            i = 0;
                        }

                        while(current_weight <= SO_CAPACITY){
                            merce_rand = rand() % SO_MERCI + 1;
                            if(arr_offerte_global[riga_matrice * (SO_MERCI+1) + merce_rand] != 0){
                                arr_offerte_global[riga_matrice * (SO_MERCI+1) + merce_rand] -= 1;
                                if(current_weight += tipi_merce[merce_rand - 1].weight <= SO_CAPACITY){
                                    Info_vita.type = (unsigned int)pos_porti[harbor_des * 3];
                                    Info_vita.operation = 4;
                                    Info_vita.extra = tipi_merce[merce_rand - 1].type;
                                    msgsnd(msg_porti_navi_id, &Info_vita, sizeof(int) * 2, 0);
                                    msg_bytes = msgrcv(msg_porti_navi_id, &Info_vita, sizeof(int) * 2, (unsigned int)pos_porti[harbor_des * 3], 0);
                                    if(msg_bytes >= 0 && Info_vita.operation == 4){
                                        temp = tipi_merce[merce_rand-1];
                                        temp.life = Info_vita.extra;
                                        stiva = list_insert(stiva, temp); 
                                    }
                                }   
                                else{
                                    current_weight -= tipi_merce[merce_rand - 1].weight;
                                }
                                
                                printf("Carico merce : %d, peso: %d, peso tot: %d\n",merce_rand, tipi_merce[merce_rand - 1].weight, current_weight);
                            }
                            else{
                                merce_rand = rand() % SO_MERCI + 1;
                            }
                            
                        }
                        
                    }
                    riga_matrice = - 1;
                }

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