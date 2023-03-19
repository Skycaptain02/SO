#define _GNU_SOURCE  /* Per poter compilare con -std=c89 -pedantic */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

int main(int argvc, char * argv[]){

    int n_figli = atoi(argv[1]); //devo ricordare che i valori passati da riga di comando sono char
    pid_t figli[n_figli]; //creo un array in cui inseriro i pid dei figli creati dalla fork per potergli mandare un segnale
    int pid_figlio;
    int status; //valore di ritrono dello statutus
    char * argv_loop[] = {"","Ciao", NULL}; //la execve vuole come primo parametro il nome dell'eseguibile, come secondo il paramentro da passare via riga di comanda, terzo nullo
    char * stringa; //creo una stringa di valori di exitstauts
    stringa = malloc(n_figli * sizeof(stringa)); //creo l'array
    
    int i = 0;
    for(i = 0; i < n_figli;  i++){
        switch(figli[i] = fork()){
            case -1:
                printf("Errore\n");
                exit(-1);
            break;
            case 0:
                printf("[FIGLIO] = %d\n", getpid());
                execve("char-loop", argv_loop, NULL);
            break;
            default:
            break;
        }
    }
    sleep(1);
    for(i = 0; i < n_figli; i++){
        kill(figli[i], SIGINT);
    }
    i = 0;
    while(wait(&status) != -1){
        stringa[i++] = WEXITSTATUS(status);
    }
    printf("La stringa ottenuta : %s\n", stringa);
}