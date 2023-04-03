#include "env_var.h"
#include "ipc.h"



int main(int argc, char * argv[]){
    
    double ship_pos_x;
    double ship_pos_y;
    int sem_config_id;

    /*vado a creare in un punto randomico della mappa la nave*/
    ship_pos_x = rand() % SO_LATO;
    ship_pos_y = rand() % SO_LATO;



    /*Sezione creazione semaforo per configurazione*/
    sem_config_id = semget(getppid(), 1, 0600 | IPC_CREAT);
    sem_reserve(sem_config_id, 0);

    /*printf("Sottraggo 1\n");*/
    /*Fine Sezione*/

}