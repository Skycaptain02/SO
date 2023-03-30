#include "env_var.h"
#include "ipc.h"



int main(int argc, char * argv[]){
    
    double ship_pos_x;
    double ship_pos_y;

    /*Sezione creazione semaforo per configurazione*/
    sem_reserve(atoi(argv[1]), 0);
    printf("Sottraggo 1\n");
    /*Fine Sezione*/

    /*vado a creare in un punto randomico della mappa la nave*/
    ship_pos_x = rand() % SO_LATO;
    ship_pos_y = rand() % SO_LATO;

    
    
    /*lancio segnale al master e rimango in attesa della creazione dei porti*/

    exit(0);
}