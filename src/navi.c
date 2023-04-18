#include "env_var.h"
#include "../lib/ipc.h"

void handler_start(int signal){
}

int main(int argc, char * argv[]){
    
    double ship_pos_x;
    double ship_pos_y;
    int sem_config_id;
    double * pos_porti;
    int shm_pos_porti_id, harbor_des;
    struct sigaction sa;

    srand(getpid());

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_start;
    sigaction(SIGUSR1, &sa, NULL);
    
    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);
    sem_reserve(sem_config_id, 0);

    pause();

    shm_pos_porti_id = shmget(getppid() + 5, sizeof(double) * (SO_PORTI * 3), 0600 | IPC_CREAT);
    pos_porti = shmat(shm_pos_porti_id, NULL, 0);  

    harbor_des = rand() % SO_PORTI;

    /*printf("[NAVE] -> HARBOR-PID: %f, HARBOR-X: %f, HARBOR-Y: %f\n", pos_porti[harbor_des * SO_PORTI], pos_porti[harbor_des * SO_PORTI + 1], pos_porti[harbor_des * SO_PORTI + 2]);*/




}