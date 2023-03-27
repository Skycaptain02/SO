#define _GNU_SOURCE  /* Per poter compilare con -std=c89 -pedantic */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>


int main(int argc, char * argv[]){
    int n_pipes = atoi(argv[1]);
    int * pipes;
    pipes = calloc(2 * n_pipes, sizeof(* pipes));
    pid_t child_pid[n_pipes];
    int p_re, p_we;
    int num_bytes;
    int buff_read;
    int number;

    srand(getpid());
    int i, j;

    for(i = 0; i < n_pipes; i++){
        pipe(pipes + 2 * i);
    }
    for(i = 0; i < n_pipes; i++){
        switch(child_pid[i] = fork()){
            case -1:
                printf("Errore\n");
            break;
            case 0:
                int j = (i  == n_pipes - 1 ? 0 : i + 1);
                p_re = pipes[j][0];
                p_we = pipes[i][1];
                for(j = 0; j < n_pipes; j++){
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                while(num_bytes = read(p_re, &buff_read, sizeof(buff_read))){
                    if(!buff_read){
                        write(p_we, &buff_read, sizeof(buff_read));  
                    }
                    else{
                        buff_read--;
                        write(p_we, &buff_read, sizeof(buff_read));
                        printf("[FIGLIO] = Ho decrementato di 1\n");
                    }
                }
                exit(0);
            break;
            default:
        }
    }
    number = rand() % atoi(argv[2] + 1);
    write(pipes[0][1], &number, sizeof(number));
    while(wait(NULL) != -1);
    
}