#define _GNU_SOURCE    /* NECESSARY from now on */ 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <signal.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <strings.h>


void timer_handler(int signal){
    switch(signal){
        case SIGALRM:
            printf("Tempo scaduto, riprova\n");
            exit(0);
        break;
        case SIGUSR1:
            printf("ciao\n");
            break;
        default:

    }
}

int main(int argvc, char * argv[]){
    
    srand(getpid());

    struct sigaction sa;                //creo la struct sigaction chiamata sa
    bzero(&sa, sizeof(sa));             //imposto tutti i campi della struct a 0
    sa.sa_handler = timer_handler;      //referenzio il valore della sturct alla funzione handler creata
    sigaction(SIGALRM, &sa, NULL);      //aggancio il signale alla struct
    sigaction(SIGUSR1, &sa, NULL);
    int secondi = strtol(argv[2], NULL, 10);
    int a = 0;
    int s = strtol(argv[1], NULL, 10);
    int b = rand() % s;
    int guess = 0;
    
    alarm(secondi);                     //dopo n secondi lancia sigalarm
    printf("Inserisci il numero:");
    scanf("%d", &guess);

    while(guess != b){
        if(guess > b){
            printf("Il numero inserito e' troppo grande\n");
            printf("Inserisci il numero:");
            scanf("%d", &guess);
        }
        else{
            printf("Il numero inserito e' troppo piccolo\n");
            printf("Inserisci il numero:");
            scanf("%d", &guess);
        }
    }
    kill(getpid(),SIGUSR1);
    printf("Hai azzeccato il numero, sei forte, sali\n");
}