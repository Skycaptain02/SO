#include "env_var.h"



struct merci{
    int * name;
    int * weight;
    int * life;
};



int main(int argc, char * argv[]){
    
    double ship_pos_x;
    double ship_pos_y;

    /*vado a creare in un punto randomico della mappa la nave*/
    ship_pos_x = rand() % SO_LATO;
    ship_pos_y = rand() % SO_LATO;

    kill(getppid(), SIGUSR1);
    
    /*lancio segnale al master e rimango in attesa della creazione dei porti*/

    exit(0);
}