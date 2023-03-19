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

unsigned char c = ' ';

int main(int arvc, char * argv[]){
    
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = sig_handler;
    sigaction(SIGINT, &sa, NULL);
    c = * argv[1];
    
    while(1){
        c++;
        if(c >= 126){
            c = 33;
        }
    }
}

void sig_handler(int signal){
    printf("Exit(%d)\n", c);
    exit(c);
}