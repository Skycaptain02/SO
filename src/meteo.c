#include "env_var.h"
#include "../lib/ipc.h"

void dailyDisaster();

int flag_end = 0, swellIndex = 0;
pid_t * pidNavi, * pidPorti;
int * statusNavi, * portiSwell;

void handler(int signal){
    switch (signal){
    case SIGABRT:
        flag_end = 1;
        break;
    case SIGUSR1:
        dailyDisaster();
        break;
    default:
        break;
    }
}

int main(int argc, char * argv[]){
    int sem_config_id, err;
    int shm_pidNavi_id, shm_pidPorti_id, shm_statusNavi_id, shm_portiSwell_id;
    
    int i;
    int numNavi = SO_NAVI;
    struct timespec req, rem, rem2;
    struct sigaction sa;
    int modulo;
    double nsec;
    int pid_random;

    srand(getpid());
 
    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);


    if(SO_MAELESTROM < 24){
        modulo = 0;
        nsec = (((double)SO_MAELESTROM / (double)24) * CONVERSION_SEC_NSEN);
        req.tv_sec = (time_t)(modulo);
        req.tv_nsec = (long)nsec;
    }
    else{
        modulo = (int)SO_MAELESTROM / 24;
        nsec = ((double)SO_MAELESTROM / (double)24 - modulo) * CONVERSION_SEC_NSEN;
        req.tv_sec = (time_t)(modulo);
        req.tv_nsec = (long)(nsec);
    }

    /*Sezione creazione semaforo per configurazione*/
    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);
    sem_reserve(sem_config_id, 0);
    /*Fine Sezione*/

    shm_pidNavi_id = shmget(getppid() + 13, sizeof(pid_t) * SO_NAVI, 0600 | IPC_CREAT);
    pidNavi = shmat(shm_pidNavi_id, NULL, 0);

    shm_pidPorti_id = shmget(getppid() + 14, sizeof(pid_t) * SO_PORTI, 0600 | IPC_CREAT);
    pidPorti = shmat(shm_pidPorti_id, NULL, 0);

    shm_statusNavi_id = shmget(getppid() + 9, sizeof(int) * SO_NAVI * 5, 0600 | IPC_CREAT);
    statusNavi = shmat(shm_statusNavi_id, NULL, 0);

    shm_portiSwell_id = shmget(getppid() + 16, sizeof(int) * SO_DAYS, 0600 | IPC_CREAT);
    portiSwell = shmat(shm_portiSwell_id, NULL, 0);

    i = 0;
    
    while(!flag_end && numNavi > 0){
        while((req.tv_nsec != 0 || req.tv_sec != 0) && !flag_end){
            if(nanosleep(&req, &rem) == -1){
                req.tv_nsec = rem.tv_nsec;
                req.tv_sec = rem.tv_sec;
            }  
            else{
                req.tv_nsec = 0;
                req.tv_sec = 0;
            }
        }
        if(!flag_end){
            pid_random = rand() % SO_NAVI;
            while(pidNavi[pid_random] == -1){
                pid_random = rand() % SO_NAVI;
            }
            kill(pidNavi[pid_random], SIGTERM);
            pidNavi[pid_random] = - 1;
            statusNavi[(pid_random * 6) + 1] = 0;  
            statusNavi[(pid_random * 6) + 2] = 0;  
            statusNavi[(pid_random * 6) + 3] = 0;  
            statusNavi[(pid_random * 6) + 4] = 1;  
            numNavi -= 1;
            req.tv_sec = (time_t)modulo;
            req.tv_nsec = (long)nsec;
        }
        
    }
    if(numNavi == 0){
        kill(getppid(), SIGABRT);
    }

    shmdt(pidNavi);
    shmdt(pidPorti);
    shmdt(statusNavi);
    shmdt(portiSwell);
}

void dailyDisaster(){

    /**
     * SIGUSR2 -> NAVE ATTENDE ATTENDE MENTRE VIAGGIA / PORTO ATTENDE
     * SIGABRT -> MORTE DELLA NAVE
    */

    int pid_random;
    pid_random = rand() % SO_NAVI;
    while(pidNavi[pid_random] == -1){
        pid_random = rand() % SO_NAVI;
    }
    kill(pidNavi[pid_random], SIGUSR2);
    statusNavi[(pid_random * 6) + 5] += 1;
    
    pid_random = rand() % SO_PORTI;
    kill(pidPorti[pid_random], SIGUSR2);
    portiSwell[swellIndex] = pidPorti[pid_random];
    swellIndex += 1;
}