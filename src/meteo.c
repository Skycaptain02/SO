#include "env_var.h"
#include "../lib/ipc.h"

int SO_PORTI, SO_NAVI, SO_MERCI, SO_SIZE, SO_MIN_VITA, SO_MAX_VITA, SO_LATO, SO_SPEED, SO_CAPACITY, SO_BANCHINE; 
int SO_FILL, SO_LOADSPEED, SO_DAYS, SO_STORM_DURATION, SO_SWELL_DURATION, SO_MAELESTROM, PRINT_MERCI, CONVERSION_SEC_NSEN;

void readInputs();

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
    int numNavi;
    struct timespec req, rem, rem2;
    struct sigaction sa;
    int modulo;
    double nsec;
    int pid_random;

    readInputs();

    numNavi = SO_NAVI;
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