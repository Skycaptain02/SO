#include "env_var.h"
#include "../lib/ipc.h"
#include "../lib/list.h"
#include <math.h>

int SO_PORTI, SO_NAVI, SO_MERCI, SO_SIZE, SO_MIN_VITA, SO_MAX_VITA, SO_LATO, SO_SPEED, SO_CAPACITY, SO_BANCHINE; 
int SO_FILL, SO_LOADSPEED, SO_DAYS, SO_STORM_DURATION, SO_SWELL_DURATION, SO_MAELESTROM, PRINT_MERCI, CONVERSION_SEC_NSEN;

void readInputs();
void stormPause();
void funcEnd(int);
void travel(int *, double);
int harborOperations(int *, int);
int getRow(int *, double *, int );
struct MsgOp genMessaggio(unsigned int, int, int, pid_t);
double calcoloDistanza(int, double *, int, int);

List stiva , temp_stiva;
int flag_end = 0, flag_day = 0, current_weight = 0;
int * arr_richieste_global, * arr_offerte_global, * merci_consegnate, * statusNavi, * statusMerci, * merci_scadute;
Merce * tipi_merce;
double * pos_porti;
int posStatus = 0, lastHarbor = 0, flagSemCarico = 0, flagSemScarico = 0;

void handler_start(int signal){
    switch(signal){
        case SIGABRT:
            flag_end = 1;
            funcEnd(0);
        break;
        case SIGTERM:
            flag_end = 1;
            funcEnd(1);
        break;
        case SIGUSR2:
            stormPause();
            break;
        default:
            printf("ERROR\n");
        break;
    }
}

int main(int argc, char * argv[]){
    
    
    sigset_t maskBlock;
    struct sigaction sa;
    struct MsgOp Operation;
    Merce temp;
    int * richieste_local, * offerte_local;
    
    double ship_pos_x, dist_parz_x;
    double ship_pos_y, dist_parz_y;
    double distanza;

    int harbor_des;
    int sem_config_id, sem_offerte_richieste_id, sem_opPorti_id;
    int msg_porti_navi_id, msg_bytes;
    int riga_matrice = - 1;
    int harbor_des_old;

    int shm_pos_porti_id, shm_merci_id, shm_richieste_local_id, shm_offerte_local_id, shm_merci_consegnate_id, shm_statusNavi_id, shm_statusMerci_id;
    int i, merce_rand, id_merce = 1;
    int flag_end_carico = 1, flag_end_scarico = 1, flag_ctrl = 1;
    int merci_scaricate = 0, merci_caricate = 0;
    int exit = 0;
    int ernno;

    readInputs();
    srand(getpid());
   
    listCreate(&stiva);
    

    merci_scadute = malloc(sizeof(int));

    shm_merci_id = shmget(getppid() + 1, sizeof(tipi_merce) * SO_MERCI, 0600 | IPC_CREAT);
    tipi_merce = shmat(shm_merci_id, NULL, 0);

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_start;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    sem_offerte_richieste_id = semget(getppid() + 1, 1, 0600 | IPC_CREAT);
    while(semctl(sem_offerte_richieste_id, 0, GETVAL) != 0); /* ATTENDIAMO CHE I PORTI FINISCANO DI GENERARE LA PRIMA VOLTA*/

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

    shm_statusNavi_id = shmget(getppid() + 9, sizeof(int) * SO_NAVI * 6, 0600 | IPC_CREAT);
    statusNavi = shmat(shm_statusNavi_id, NULL, 0);
    
    shm_statusMerci_id = shmget(getppid() + 10, sizeof(int) * (SO_MERCI) * 5, 0600 | IPC_CREAT);
    statusMerci = shmat(shm_statusMerci_id, NULL, 0);


    i = 0;
    while(statusNavi[i * 6] != getpid()){
        i++;
    }
    posStatus = i;
    statusNavi[(posStatus * 6) + 1] = 1;
    statusNavi[(posStatus * 6) + 2] = 0;
    statusNavi[(posStatus * 6) + 3] = 0;
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
                lastHarbor = (unsigned int)pos_porti[harbor_des * 3];
                sigemptyset(&maskBlock);
                sigaddset(&maskBlock, SIGTERM);
                sigaddset(&maskBlock, SIGUSR2);
                sigprocmask(SIG_BLOCK, &maskBlock, NULL);
                sem_opPorti_id = semget(lastHarbor, 2, 0600 | IPC_CREAT);
                if(current_weight > 0){           
                    riga_matrice = getRow(arr_richieste_global, pos_porti, harbor_des);
                    flag_end_scarico = 1;
                    temp_stiva.top = stiva.top;
                    while(temp_stiva.top != NULL){
                        sem_reserve(sem_opPorti_id, 0);       
                        flagSemScarico = 1;
                        if(arr_richieste_global[riga_matrice * (SO_MERCI + 1) + temp_stiva.top->elem.type] != 0){
                            arr_richieste_global[riga_matrice * (SO_MERCI + 1) + temp_stiva.top->elem.type] -= 1;
                            sem_release(sem_opPorti_id, 0);
                            flagSemScarico = 0;
                            Operation = genMessaggio((unsigned int)pos_porti[harbor_des * 3], 1, temp_stiva.top->elem.type, getpid());
                            msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t));
                            msg_bytes = msgrcv(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), getpid(), 0);
                            current_weight -= temp_stiva.top->elem.weight;
                            listRemoveToLeft(&stiva, NULL, temp_stiva.top->elem.type);
                            merci_consegnate[temp_stiva.top->elem.type - 1] += 1;
                            merci_scaricate += 1;
                            
                        }else{
                            sem_release(sem_opPorti_id, 0);
                            flagSemScarico = 0;
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
                                if(i >= SO_MERCI + 1){
                                    id_merce = 1;
                                    i = 1;
                                }
                                sem_reserve(sem_opPorti_id, 1);
                                flagSemCarico = 1;
                                if(arr_offerte_global[riga_matrice * (SO_MERCI + 1) + i] != 0){
                                    arr_offerte_global[riga_matrice * (SO_MERCI + 1) + id_merce] -= 1;
                                    sem_release(sem_opPorti_id, 1);
                                    flagSemCarico = 0;
                                    flag_ctrl = 0;
                                    id_merce = i;
                                }
                                else{
                                    sem_release(sem_opPorti_id, 1);
                                    flagSemCarico = 0;
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
                            current_weight += tipi_merce[id_merce - 1].weight;
                            if(current_weight <= SO_CAPACITY){
                                Operation = genMessaggio((unsigned int)pos_porti[harbor_des * 3], 2, tipi_merce[id_merce - 1].type, getpid());
                                msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), 0);
                                msg_bytes = msgrcv(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), getpid(), 0);
                                if(msg_bytes >= 0 && Operation.operation == 3){
                                    temp = tipi_merce[id_merce - 1];
                                    temp.life = Operation.extra;
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
                    exit = harborOperations(statusNavi ,merci_caricate);
                    merci_caricate = 0;
                }
                
                if(!exit){
                    Operation = genMessaggio((unsigned int)pos_porti[harbor_des * 3], 4 , 0, getpid());
                    msgsnd(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), 0);
                    msg_bytes = msgrcv(msg_porti_navi_id, &Operation, sizeof(int) * 2 + sizeof(pid_t), getpid(), 0);
                    riga_matrice = - 1;
                } 

                sigemptyset(&maskBlock);
                sigaddset(&maskBlock, SIGTERM);
                sigaddset(&maskBlock, SIGUSR2);
                sigprocmask(SIG_UNBLOCK, &maskBlock, NULL);
                sem_opPorti_id = 0;
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

void travel(int * statusNavi, double distanza){
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
    if(current_weight == 0){
        statusNavi[(posStatus * 6) + 1] = 1;
        statusNavi[(posStatus * 6) + 2] = 0;
        statusNavi[(posStatus * 6) + 3] = 0;
    }
    else{
        statusNavi[(posStatus * 6) + 1] = 0;
        statusNavi[(posStatus * 6) + 2] = 1;
        statusNavi[(posStatus * 6) + 3] = 0;
    }
    while(req.tv_nsec != 0 || req.tv_sec != 0){
        if(nanosleep(&req, &rem) == -1){
            req.tv_nsec = rem.tv_nsec;
            req.tv_sec = rem.tv_sec;
        }  
        else{
            req.tv_nsec = 0;
            req.tv_sec = 0;
        }
    }
    if(stiva.top != NULL){
        for(i = 0; i < modulo; i++){
            listSubtract(&stiva, merci_scadute, statusMerci, 0);
        }
    }
}

int harborOperations(int * statusNavi, int quantity){
    struct timespec req, rem;
    int modulo;
    int i = 0;
    int exit = 0;
    double nsec, waitTime;
    rem.tv_nsec = 0;
    rem.tv_sec = 0;

    statusNavi[(posStatus * 6) + 1] = 0;
    statusNavi[(posStatus * 6) + 2] = 0;
    statusNavi[(posStatus * 6) + 3] = 1;
    
    waitTime = (double)quantity/(double)SO_LOADSPEED;
    modulo = (int)waitTime;
    nsec = (waitTime - modulo) * CONVERSION_SEC_NSEN;
    req.tv_sec = (time_t)(modulo);
    req.tv_nsec = (long)(nsec);

    nanosleep(&req, &rem);
    if(rem.tv_nsec != 0 || rem.tv_sec != 0){
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

void funcEnd(int flag){
    int i, id_opPorti_id;

    if(flag){
        statusNavi[(posStatus * 6) + 1] = 0;
        statusNavi[(posStatus * 6) + 2] = 0;
        statusNavi[(posStatus * 6) + 3] = 0;
    }

    if(!flag){
        id_opPorti_id = semget(lastHarbor, 2, 0600 | IPC_CREAT);
        if(flagSemScarico){
            sem_release(id_opPorti_id, 0);
        }
        if(flagSemCarico){
            sem_release(id_opPorti_id, 1);
        }
    }
    
    shmdt(tipi_merce);
    shmdt(pos_porti);
    shmdt(merci_consegnate);
    shmdt(arr_richieste_global);
    shmdt(arr_offerte_global);
    shmdt(statusNavi);
    shmdt(statusMerci);

    free(merci_scadute);

    listFree(&stiva);
    listFree(&temp_stiva);

    exit(0);
}

void stormPause(){
    struct timespec req, rem;
    int modulo;
    int i = 0;
    int exit = 0;
    double nsec, waitTime;
    sigset_t maskBlock;
    rem.tv_nsec = 0;
    rem.tv_sec = 0;

    sigemptyset(&maskBlock);
    sigaddset(&maskBlock, SIGTERM);
    sigaddset(&maskBlock, SIGUSR2);
    sigprocmask(SIG_BLOCK, &maskBlock, NULL);
    if(SO_STORM_DURATION < 24){
        modulo = 0;
        nsec = (((double)SO_STORM_DURATION / (double)24) * CONVERSION_SEC_NSEN);
        req.tv_sec = (time_t)(modulo);
        req.tv_nsec = (long)nsec;
    }
    else{
        modulo = (int)SO_STORM_DURATION / 24;
        nsec = ((double)SO_STORM_DURATION / (double)24 - modulo) * CONVERSION_SEC_NSEN;
        req.tv_sec = (time_t)(modulo);
        req.tv_nsec = (long)(nsec);
    }

    nanosleep(&req, &rem);

    sigemptyset(&maskBlock);
    sigaddset(&maskBlock, SIGTERM);
    sigaddset(&maskBlock, SIGUSR2);
    sigprocmask(SIG_UNBLOCK, &maskBlock, NULL);
}

void readInputs(){
    const char * file_path = "../src/env_var.txt";
    char buffer[1024];
    char buffer_cpy[1024];
    ssize_t bytes_read;
    char * token;
    FILE * file_descriptor = fopen(file_path, "r");


    if (!file_descriptor) {
        perror("Failed to open the file");
    }

    while (fgets(buffer, 1024, file_descriptor)){
        token = strtok(buffer, " ");
        if(strcmp(token, "SO_NAVI") == 0){
            token = strtok(NULL, " ");
            SO_NAVI = atoi(token);
        }else if(strcmp(token, "SO_PORTI") == 0){
            token = strtok(NULL, " ");
            SO_PORTI = atoi(token);
        }else if(strcmp(token, "SO_MERCI") == 0){
            token = strtok(NULL, " ");
            SO_MERCI = atoi(token);
        }else if(strcmp(token, "SO_SIZE") == 0){
            token = strtok(NULL, " ");
            SO_SIZE = atoi(token);
        }else if(strcmp(token, "SO_MIN_VITA") == 0){
            token = strtok(NULL, " ");
            SO_MIN_VITA = atoi(token);
        }else if(strcmp(token, "SO_MAX_VITA") == 0){
            token = strtok(NULL, " ");
            SO_MAX_VITA = atoi(token);
        }else if(strcmp(token, "SO_LATO") == 0){
            token = strtok(NULL, " ");
            SO_LATO = atoi(token);
        }else if(strcmp(token, "SO_SPEED") == 0){
            token = strtok(NULL, " ");
            SO_SPEED = atoi(token);
        }else if(strcmp(token, "SO_CAPACITY") == 0){
            token = strtok(NULL, " ");
            SO_CAPACITY = atoi(token);
        }else if(strcmp(token, "SO_BANCHINE") == 0){
            token = strtok(NULL, " ");
            SO_BANCHINE = atoi(token);
        }else if(strcmp(token, "SO_FILL") == 0){
            token = strtok(NULL, " ");
            SO_FILL = atoi(token);
        }else if(strcmp(token, "SO_LOADSPEED") == 0){
            token = strtok(NULL, " ");
            SO_LOADSPEED = atoi(token);
        }else if(strcmp(token, "SO_DAYS") == 0){
            token = strtok(NULL, " ");
            SO_DAYS = atoi(token);
        }else if(strcmp(token, "SO_STORM_DURATION") == 0){
            token = strtok(NULL, " ");
            SO_STORM_DURATION = atoi(token);
        }else if(strcmp(token, "SO_SWELL_DURATION") == 0){
            token = strtok(NULL, " ");
            SO_SWELL_DURATION = atoi(token);
        }else if(strcmp(token, "SO_MAELESTROM") == 0){
            token = strtok(NULL, " ");
            SO_MAELESTROM = atoi(token);
        }else if(strcmp(token, "PRINT_MERCI") == 0){
            token = strtok(NULL, " ");
            PRINT_MERCI = atoi(token);
        }else if(strcmp(token, "CONVERSION_SEC_NSEN") == 0){
            token = strtok(NULL, " ");
            CONVERSION_SEC_NSEN = atoi(token);
        }
    }
    fclose(file_descriptor);
}