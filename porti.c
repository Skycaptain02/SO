#include "env_var.h"
#include "ipc.h"

int main(int argc, char * argv[]){
    /*Sezione creazione semaforo per configurazione*/
    sem_reserve(atoi(argv[1]), 0);
    printf("Sottraggo 1\n");
    /*Fine Sezione*/
}