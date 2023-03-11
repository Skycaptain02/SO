
#define _GNU_SOURCE    /* NECESSARY from now on */ 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <signal.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <strings.h>

void timer_handler(int signal) {
    switch (signal)
    {
        case SIGALRM:
            printf("Tempo scaduto\n");
            exit(0);
            break;

        case SIGUSR1:
            printf("sigusr\n");
        break; 

        case 45:
            printf("123\n");
        break;
        default:
            break;
    }
	
}

int main(int argvc, char * argv[]){
    srand(getpid());

    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = timer_handler;
    sigaction(SIGALRM, &sa, NULL); //GENERAZIONE DI STRUCT
    sigaction(SIGUSR1, &sa, NULL); //GENERAZIONE DI STRUCT
    sigaction(45, &sa, NULL); //GENERAZIONE DI STRUCT

    int r_start = strtol(argv[1], NULL, 10);

    int num_S = strtol(argv[2], NULL, 10);

    int rand_num = rand() % r_start;

    int guess  = -1 ;

    alarm(num_S);
    
    while(guess != rand_num)
    {
        printf("Inserisci un numero: ");
            scanf("%d", &guess);
            printf("\n");

        if(guess > rand_num){
            printf("Il numero è troppo grande\n");
            
        }else if(guess < rand_num){
            printf("Il numero è troppo piccolo\n");
        }
    }

    kill(getpid(),45);
    printf("Hai indovinato il numero\n");
    
    

}

