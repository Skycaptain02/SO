#define _GNU_SOURCE  /* Per poter compilare con -std=c89 -pedantic */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char * argv[]){
    int n_kids = atoi(argv[1]);
    int pipes[n_kids][2];
    int i = 0;
    int buf_read;
    pid_t pid;
    int p_re, p_we, num_bytes;

    srand(getpid());

    for(i = 0; i < n_kids; i++){
        pipe(pipes[i]);
    }

    for (i = 0; i < n_kids; i++)
    {
        switch (pid = fork())
        {
        case -1:
            printf("Error\n");
            break;
        case 0:
            int j = (i == n_kids-1 ? 0: i+1);
            p_re = pipes[i][0];
            p_we = pipes[j][1];
             sleep(1);
            for(int k = 0; k < n_kids; k++){
                if(pipes[k][0] != p_re && pipes[k][1] != p_we){
                    close(pipes[k][0]);
                    close(pipes[k][1]);
                }
                
            }

            while(num_bytes = read(p_re, &buf_read, sizeof(buf_read))){
                printf("PID %d: read %d bytes, value=%d\n", getpid(), num_bytes, buf_read);
                if(!buf_read){ //Caso letto zero
                    write(p_we, &buf_read, sizeof(buf_read));
                    break;
                }
                else{ //Caso letto > zero
                    buf_read--;
                    write(p_we, &buf_read, sizeof(buf_read));
                }
            }
            exit(0);
            break;
        
        default:
            break;
        }
    }

    int rand_num = rand() % atoi(argv[2]) + 1;
    write(pipes[0][1], &rand_num, sizeof(rand_num));
    for(int j = 0; j < n_kids; j++){
        close(pipes[j][0]);
        close(pipes[j][1]);
    }

    pid_t pid_status;
    while((pid_status = wait(NULL)) != -1){
        printf("Il figlio con pid: %d ha terminato\n", pid_status);
    }
    
    
}