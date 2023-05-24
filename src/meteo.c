#include "env_var.h"
#include "../lib/ipc.h"

#define CONVERSION_SEC_NSEN 1000000000

void dailyDisaster();

int flag_end = 1;
pid_t * pidNavi, * pidPorti;

void handler(int signal){
    switch (signal)
    {
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
    int a;
    int sem_config_id, err;
    int shm_pidNavi_id, shm_pidPorti_id;
    
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

    if(SO_MAELESTROM < 24){
        modulo = 0;
        nsec = (SO_MAELESTROM / 24) * CONVERSION_SEC_NSEN;
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

    while(flag_end && numNavi > 0){
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
        pid_random = rand() % SO_NAVI;
        while(pidNavi[pid_random] == -1){
            pid_random = rand() % SO_NAVI;
        }
        kill(pidNavi[pid_random], SIGABRT);
        pidNavi[i] = - 1;
        numNavi -= 1;
        req.tv_sec = (time_t)modulo;
        req.tv_nsec = (long)nsec;
    }
}

void dailyDisaster(){
    /**
     * SIGSTOP -> NAVE ATTENDE ATTENDE MENTRE VIAGGIA / PORTO ATTENDE
     * SIGABRT -> NAVE ESPLODE
    */ 
    int pid_random;
    pid_random = rand() % SO_NAVI;
    while(pidNavi[pid_random] == -1){
        pid_random = rand() % SO_NAVI;
    }
    kill(pidNavi[pid_random], SIGSTOP);
    
    pid_random = rand() % SO_PORTI;
    kill(pidPorti[pid_random], SIGSTOP);
}