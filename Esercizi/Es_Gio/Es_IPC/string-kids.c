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

int main(int argc, char * argv[]){
    int figli;
    int errno;

    figli  = atoi(argv[1]);

    int i = 0;
    pid_t pid[figli];
    int status;
    char * stringa;
    stringa = malloc(figli*sizeof(stringa));
    char * argv_loop[] = {"./char-loop","aiao", (char*)0};

    for(i = 0; i < figli; i++){
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

    sleep(1);

    for(i = 0; i < figli; i++){
        
        kill(pid[i], SIGINT);
        printf("mando sigint a figlio: %d\n", pid[i]);
    }

    i = 0;
    pid_t child_pid;
    while((child_pid = wait(&status)) != -1){
        printf("PID= %5d (PARENT): Got info of child with PID=%d, status=%d\n", getpid(), child_pid, WEXITSTATUS(status));
        stringa[i++] = WEXITSTATUS(status);
    }

    printf("Stringa ottenuta : %s\n", stringa);

    
}