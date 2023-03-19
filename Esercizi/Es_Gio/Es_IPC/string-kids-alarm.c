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


void sig_handler(int signal);

int gbl_figli;
pid_t * pid;
char * stringa;
int k = 0, rand_id;
    

int main(int argc, char * argv[]){
    int errno;
    int s_id;
    srand(getpid());
    

    gbl_figli  = atoi(argv[1]);    

    stringa = malloc(gbl_figli*sizeof(stringa));

    int i = 0;
    pid = malloc(sizeof(pid)*gbl_figli);
    int status;
    
    char * argv_loop[] = {"./char-loop","aiao", (char*)0};

    struct sigaction sa;

    bzero(&sa, sizeof(sa));

    sa.sa_handler = sig_handler;  

    sigaction(SIGALRM, &sa, NULL);


    for(i = 0; i < gbl_figli; i++){
        switch (pid[i] = fork())
        {
        case -1:
            printf("ERORE");
            exit(-1);
            break;

        case 0://Caso figlio
                printf("child pid: %d\n", getpid());
                execve("char-loop", argv_loop, NULL);
            break;
        
        default://Caso padre
            break;
        }
    }

    alarm(1);
    //sleep(1);

    /*for(i = 0; i < gbl_figli; i++){
        
        kill(pid[i], SIGINT);
        printf("mando sigint a figlio: %d\n", pid[i]);
    }*/
    while(wait(&status) != -1 || errno == EINTR){
        //stringa[i++] = WEXITSTATUS(status);
        pid[rand_id] = fork();
        if(pid[rand_id] == 0){
            execve("char-loop", argv_loop, NULL);
        }
        else{
            alarm(1);
        }
    }
    if(errno){
        printf("Error %s\n", strerror(errno));
    }

    
}

void sig_handler(int signal){
    
    rand_id = rand() % gbl_figli;
    

    kill(pid[rand_id],SIGINT);
    printf("Child killed: %d\n", pid[rand_id]);
    printf("Random num: %d\n", rand_id);

    pid_t child_pid;
    int status;

    while((child_pid = wait(&status)) != pid[rand_id]);
    stringa[k++] = WEXITSTATUS(status);
    printf("Stringa ottenuta : %s\n", stringa);
    int i, sum = 0;
    for(i = 0; i < k; i++){
        sum += stringa[i];
    }
    printf("Modulo 256: %d\n", sum%256);
    if(sum % 256 == 0){
        for(i = 0; i < gbl_figli; i++){
        
            kill(pid[i], SIGINT);
            printf("mando sigint a figlio: %d\n", pid[i]);
        }
        while(wait(NULL) != -1);
        exit(0);
    }

}