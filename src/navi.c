#include "env_var.h"
#include "../lib/ipc.h"
#include "../lib/list.h"
#include <math.h>

#define CONVERSION_SEC_NSEN 1000000000

void travel(int *, double);
struct MsgOp genMessaggio(unsigned int, int, int, pid_t);
int getRow(int *, double *, int );
double calcoloDistanza(int, double *, int, int);
int harborOperations(int *, int);
void funcEnd();


List stiva;
int flag_end = 0, flag_day = 0, current_weight = 0;
int * arr_richieste_global, * arr_offerte_global, * merci_consegnate, * statusNavi, * statusMerci, * merci_scadute;
Merce * tipi_merce;
double * pos_porti;

void handler_start(int signal){
    switch(signal){
        case SIGABRT:
            flag_end = 1;
            printf("CIAO CIAO\n");
            funcEnd();
        break;
        default:
            printf("ERROR\n");
        break;
    }
}

int main(int argc, char * argv[]){
   
    int errno;
    
    double ship_pos_x, dist_parz_x;
    double ship_pos_y, dist_parz_y;
    double distanza;

    int harbor_des;
    int sem_config_id, sem_offerte_richieste_id, shm_merci_id;

    int msg_porti_navi_id;
    int msg_bytes;
    int riga_matrice = - 1;
    int harbor_des_old;

    int shm_pos_porti_id, shm_richieste_local_id, shm_offerte_local_id, shm_merci_consegnate_id, shm_statusNavi_id, shm_statusMerci_id;
    int * richieste_local, * offerte_local;
    struct sigaction sa;
    struct MsgOp Operation, Info_vita;
    int i, merce_rand, id_merce = 1;
    int flag_end_carico = 1, flag_end_scarico = 1, flag_ctrl = 1;
    int merci_scaricate = 0, merci_caricate = 0;
    int exit = 0;
    Merce temp;
    List temp_stiva;

    listCreate(&stiva);
    
    srand(getpid());

    merci_scadute = malloc(sizeof(int));

    shm_merci_id = shmget(getppid() + 1, sizeof(tipi_merce) * SO_MERCI, 0600 | IPC_CREAT);
    tipi_merce = shmat(shm_merci_id, NULL, 0);

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_start;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGABRT, &sa, NULL);

    sem_offerte_richieste_id = semget(getppid() + 1, 1, 0600 | IPC_CREAT);
    while(semctl(sem_offerte_richieste_id, 0, GETVAL) != 0);

    shm_pos_porti_id = shmget(getppid() + 4, sizeof(double) * (SO_PORTI * 3), 0600 | IPC_CREAT);
    pos_porti = shmat(shm_pos_porti_id, NULL, 0);  

    shm_merci_consegnate_id = shmget(getppid() + 8, sizeof(int) * (SO_MERCI), 0600 | IPC_CREAT);
    merci_consegnate = shmat(shm_merci_consegnate_id, NULL, 0);

    harbor_des = rand() % SO_PORTI;
    ship_pos_x = (rand() % (SO_LATO + 1)) - (SO_LATO / 2);
    ship_pos_y = (rand() % (SO_LATO + 1)) - (SO_LATO / 2);

    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);
    sem_reserve(sem_config_id, 0);

    msg_porti_navi_id = msgget(getppid() , 0600 | IPC_CREAT);

    shm_richieste_local_id = shmget(getppid() + 6, sizeof(int) * ((SO_MERCI +1 ) * SO_PORTI), 0600);
    arr_richieste_global = shmat(shm_richieste_local_id, NULL, 0);

    shm_offerte_local_id = shmget(getppid() + 7, sizeof(int) * ((SO_MERCI + 1) * SO_PORTI), 0600);
    arr_offerte_global = shmat(shm_offerte_local_id, NULL, 0);

    shm_statusNavi_id = shmget(getppid() + 9, sizeof(int) * SO_NAVI * 4, 0600 | IPC_CREAT);
    statusNavi = shmat(shm_statusNavi_id, NULL, 0);
    
    shm_statusMerci_id = shmget(getppid() + 10, sizeof(int) * (SO_MERCI) * 5, 0600 | IPC_CREAT);
    statusMerci = shmat(shm_statusMerci_id, NULL, 0);

    i = 0;
    while(statusNavi[i * 4] != getpid()){
        i++;
    }    
    statusNavi[(i * 4) + 1] = 1;
    statusNavi[(i * 4) + 2] = 0;
    statusNavi[(i * 4) + 3] = 0;
    i = 0;

    distanza = calcoloDistanza(harbor_des, pos_porti, ship_pos_x, ship_pos_y);
    travel(statusNavi, distanza);
    ship_pos_x = pos_porti[harbor_des * 3 + 1];
    ship_pos_y = pos_porti[harbor_des * 3 + 2];

    while(!flag_end){
        Operation = genMessaggio((unsigned int)pos_porti[harbor_des * 3], 0, 0, getpid());
        msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), 0);
        msg_bytes = msgrcv(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), getpid(), 0);
        if(msg_bytes >= 0){
            if(Operation.operation == 0){
                if(current_weight > 0){           
                    riga_matrice = getRow(arr_richieste_global, pos_porti, harbor_des);
                    flag_end_scarico = 1;
                    temp_stiva.top = stiva.top;
                    while(temp_stiva.top != NULL){
                        if(arr_richieste_global[riga_matrice * (SO_MERCI + 1) + temp_stiva.top->elem.type] != 0){
                            arr_richieste_global[riga_matrice * (SO_MERCI + 1) + temp_stiva.top->elem.type] -= 1;
                            Operation = genMessaggio((unsigned int)pos_porti[harbor_des * 3], 1, temp_stiva.top->elem.type, getpid());
                            msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t));
                            msg_bytes = msgrcv(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), getpid(), 0);
                            if(msg_bytes >= 0){
                                /*printf("MERCE SCARICATA [%d]: TIPO: %d, VITA: %d, PESO: %d\n", getpid(), temp_stiva.top->elem.type, temp_stiva.top->elem.life, temp_stiva.top->elem.weight);*/
                            }
                            current_weight -= temp_stiva.top->elem.weight;
                            listRemoveToLeft(&stiva, NULL, temp_stiva.top->elem.type);
                            merci_consegnate[temp_stiva.top->elem.type - 1] += 1;
                            merci_scaricate += 1;
                            
                        }
                        temp_stiva.top = temp_stiva.top->next;
                    }
                    exit = harborOperations(statusNavi, merci_scaricate);
                    merci_scaricate = 0;
                }
                if(current_weight < SO_CAPACITY && !exit){
                    if(riga_matrice == -1){
                        riga_matrice = getRow(arr_offerte_global, pos_porti, harbor_des);
                    }
                    flag_end_carico = 1;
                    while(flag_end_carico){
                        flag_ctrl = 1;
                        if(SO_MERCI != 1){
                            for(i = id_merce; current_weight <= SO_CAPACITY && flag_ctrl == 1; i++){
                                if(i >= SO_MERCI+1){
                                    id_merce = 1;
                                    i = 1;
                                }
                                if(arr_offerte_global[riga_matrice * (SO_MERCI + 1) + i] != 0){
                                    flag_ctrl = 0;
                                    id_merce = i;
                                }
                                
                            }
                        }else{
                            if(arr_offerte_global[riga_matrice * (SO_MERCI + 1) + 1] != 0){
                                id_merce = 1;
                                flag_ctrl = 0;
                            }
                            else{
                                flag_ctrl = 1;
                            }
                        }
                        if(!flag_ctrl){
                            arr_offerte_global[riga_matrice * (SO_MERCI + 1) + id_merce] -= 1;
                            current_weight += tipi_merce[id_merce - 1].weight;
                            if(current_weight <= SO_CAPACITY){
                                Operation.type = (unsigned int)pos_porti[harbor_des * 3];
                                Operation.operation = 2;
                                Operation.extra = tipi_merce[id_merce - 1].type;
                                Operation.pid_nave = getpid();
                                msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), 0);
                                msg_bytes = msgrcv(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), getpid(), 0);
                                if(msg_bytes >= 0 && Operation.operation == 3){
                                    temp = tipi_merce[id_merce - 1];
                                    temp.life = Operation.extra;
                                    /*printf("MERCE INSERITA [%d]: TIPO: %d, VITA: %d, PESO: %d\n", getpid(), temp.type, temp.life, temp.weight);*/
                                    listInsert(&stiva, temp);
                                    merci_caricate += 1;
                                }
                            } 
                            else{
                                current_weight -= tipi_merce[id_merce - 1].weight;
                                arr_offerte_global[riga_matrice * (SO_MERCI+1) + id_merce] += 1;
                                flag_end_carico = 0;
                            }
                        }
                        else{
                            flag_end_carico = 0;
                        }
                        id_merce += 1;
                    }
                    exit = harborOperations(statusNavi 
                    ,merci_caricate);
                    merci_caricate = 0;
                }
                
                if(!exit){
                    Operation = genMessaggio((unsigned int)pos_porti[harbor_des * 3], 4 , 0, getpid());
                    msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), 0);
                    msg_bytes = msgrcv(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), getpid(), 0);
                    riga_matrice = - 1;
                } 
            }
        }
        if(!exit){
            harbor_des_old = harbor_des;
            while(harbor_des_old == harbor_des){
                harbor_des = rand() % SO_PORTI;
            }
            distanza = calcoloDistanza(harbor_des, pos_porti, ship_pos_x, ship_pos_y);
            travel(statusNavi, distanza);
            ship_pos_x = pos_porti[harbor_des * 3 + 1];
            ship_pos_y = pos_porti[harbor_des * 3 + 2];
        }
    }
}

void travel(int * status, double distanza){
    sigset_t mask_block, mask_unblock;
    struct timespec req, rem;
    int modulo; 
    int i = 0;
    double nsec;
    rem.tv_nsec = 0;
    rem.tv_sec = 0;
    
    if(distanza < SO_SPEED){
        modulo = 0;
        nsec = (distanza / SO_SPEED) * CONVERSION_SEC_NSEN;
        req.tv_sec = (time_t)(modulo);
        req.tv_nsec = (long)nsec;
    }
    else{
        modulo = (int)distanza / SO_SPEED;
        nsec = ((distanza / SO_SPEED) - modulo) * CONVERSION_SEC_NSEN;
        req.tv_sec = (time_t)(modulo);
        req.tv_nsec = (long)(nsec);
    }

    while(status[i * 4] != getpid()){
        i++;
    }    
    if(current_weight == 0){
        status[(i * 4) + 1] = 1;
        status[(i * 4) + 2] = 0;
        status[(i * 4) + 3] = 0;
    }
    else{
        status[(i * 4) + 1] = 0;
        status[(i * 4) + 2] = 1;
        status[(i * 4) + 3] = 0;
    }
    /*printf("[PID %d] DEVO ASPETTARE -> %ld SECONDI \n", getpid(), req.tv_sec);
    printf("[PID %d] DEVO ASPETTARE -> %ld N_SECONDI \n", getpid(), req.tv_nsec);*/
    if(nanosleep(&req, &rem) == - 1){
        printf("ERRORE NANOSLEEPPPPPPPPPPPPPPPPPPP\n");
    }
    if(rem.tv_nsec != 0 || rem.tv_sec != 0){
        printf("SOSPESOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n");
        printf("TEMPO RIMASTO SEC -> %ld\n", rem.tv_sec);
        printf("TEMPO RIMASTO N_SEC -> %ld\n", rem.tv_nsec);
        flag_end = 1;
    }
    if(stiva.top != NULL){
        for(i = 0; i < modulo; i++){
            listSubtract(&stiva, merci_scadute, statusMerci, 0);
        }
    }
}

int harborOperations(int * status, int quantity){
    struct timespec req, rem;
    int modulo;
    int i = 0;
    int exit = 0;
    double nsec, waitTime;
    rem.tv_nsec = 0;
    rem.tv_sec = 0;

    while(status[i * 4] != getpid()){
        i++;
    }
    status[(i * 4) + 1] = 0;
    status[(i * 4) + 2] = 0;
    status[(i * 4) + 3] = 1;
    
    waitTime = (double)quantity/(double)SO_LOADSPEED;
    modulo = (int)waitTime;
    nsec = (waitTime - modulo) * CONVERSION_SEC_NSEN;
    req.tv_sec = (time_t)(modulo);
    req.tv_nsec = (long)(nsec);

    
    

    /*printf("[PID %d] TEMPO OPERAZIONI -> %ld SECONDI \n", getpid(), req.tv_sec);
    printf("[PID %d] TEMPO OPERAZIONI -> %ld N_SECONDI \n", getpid(), req.tv_nsec);*/

    if(nanosleep(&req, &rem) == - 1){
        printf("ERRORE NANOSLEEPPPPPPPPPPPPPPPPPPP HARBORO\n");
    }
    if(rem.tv_nsec != 0 || rem.tv_sec != 0){
        printf("SOSPESOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO HARBOR OP\n");
        printf("TEMPO RIMASTO SEC HARBOR OP -> %ld\n", rem.tv_sec);
        printf("TEMPO RIMASTO N_SEC HARBOR OP -> %ld\n", rem.tv_nsec);
        exit = 1;
        flag_end = 1;
    }
    return exit;
}

struct MsgOp genMessaggio(unsigned int type, int operation, int extra, pid_t pid_nave){
    struct MsgOp temp;
    temp.type = (long)type;
    temp.operation = operation;
    temp.extra = extra;
    temp.pid_nave = pid_nave;
    return temp;
}

int getRow(int * arr_richieste_global, double * pos_porti, int harbor_des){
    int i = 0;
    while(arr_richieste_global[i * (SO_MERCI + 1)] != (unsigned int)pos_porti[harbor_des * 3]){
        i++;
    }
    return i;
}

double calcoloDistanza(int harbor_des, double * pos_porti, int ship_pos_x, int ship_pos_y){
    double dist_parz_x, dist_parz_y, distanza;
    dist_parz_x = pow((pos_porti[harbor_des * 3 + 1] - ship_pos_x), 2);
    dist_parz_y = pow((pos_porti[harbor_des * 3 + 2] - ship_pos_y), 2);
    distanza = sqrt(dist_parz_x+dist_parz_y);
    return distanza;
}

void funcEnd(){

    shmdt(tipi_merce);
    shmdt(pos_porti);
    /*shmdt(merci_consegnate);*/
    shmdt(arr_richieste_global);
    shmdt(arr_offerte_global);
    shmdt(statusNavi);
    shmdt(statusMerci);
    
    exit(0);
}
