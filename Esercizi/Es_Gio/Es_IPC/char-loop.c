#define _GNU_SOURCE  /* Per poter compilare con -std=c89 -pedantic */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

void Sig_Handler(int signal);
unsigned char c =  ' ';

int main(int argc, char * argv[]){
    
    struct sigaction sa;

    bzero(&sa, sizeof(sa));

    sa.sa_handler = Sig_Handler;
    
    int s_sem = atoi(argv[2]);

    int id_sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);

    struct  sembuf my_op;

    my_op.sem_num = 0;
    my_op.sem_op = -1;
    semop(id_sem, &my_op, 1);

    sigaction(SIGINT, &sa, NULL);

    

    c = *argv[1];

    while(1){
        c++;
        if(c == 126){
            c = 33;
        }
    }


}

void Sig_Handler(int signal){
    dprintf(1,"Valore corrente di C: %d\n", c);

    exit(c);
}