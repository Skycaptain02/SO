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

void sig_Handler(int signal);

int gbl_figli;
pid_t * figli;
char * stringa;
int k = 0;
int rand_id;

int main(int argvc, char * argv[]){
    
    srand(getpid());

    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = sig_Handler;
    sigaction(SIGALRM, &sa, NULL);

    gbl_figli = atoi(argv[1]); //devo ricordare che i valori passati da riga di comando sono char
    figli = malloc(sizeof(figli) * gbl_figli); //creo un array in cui inseriro i pid dei figli creati dalla fork per potergli mandare un segnale
    char * argv_loop[] = {"","Ciao", NULL}; //la execve vuole come primo parametro il nome dell'eseguibile, come secondo il paramentro da passare via riga di comanda, terzo nullo
    stringa = malloc(gbl_figli * sizeof(stringa)); //creo l'array
    int status;
    
    int i = 0;

    for(i = 0; i < gbl_figli;  i++){
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

    alarm(1);
    while(wait(&status)!= -1 || errno == EINTR){
        figli[rand_id] = fork();
        if(figli[rand_id] == 0){
            execve("char-loop", argv_loop, NULL);
        }
        else{
            alarm(1);
        }
    }
}

void sig_Handler(int signal){
    int status; 
    int i;
    int sum = 0;

    rand_id = rand() % gbl_figli;
    printf("Il figlio scelto e' %d\n", figli[rand_id]);
    kill(figli[rand_id], SIGINT);

    while(wait(&status) != figli[rand_id]); //aspetto che il figlio r termini

    stringa[k++] = WEXITSTATUS(status);
    printf("[STRINGA] = %s\n", stringa);
    for(i = 0; i < k; i++){
        sum = sum + stringa[i];
    }
    printf("La somma ottenuta e' = %d\n", sum);
    if(sum % 256 == 0){
        for(i = 0; i < gbl_figli; i++){
            kill(figli[i], SIGINT);
        }
        while(wait(NULL) != -1);
        exit(0);
    }
}