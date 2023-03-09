#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

//la fork restituisce 0 se sei nel figlio, altrimenti sei nel caso padre
int main(int argc, char * argv[]){
    char * str;
    str = argv[1];
    int n_kids = strtol(argv[2],NULL,10);
    int n_writes = strtol(argv[3],NULL,10);
    int fd = open(str, O_WRONLY);
    int child_id;
    for(int i = 0; i < n_kids; i++){
        child_id = fork();
        //sono un processo figlio, child_id > 0, devo andare a stampare una volta che sono stato creato
        if(child_id){
            break;
        }
    }
    if(child_id != 0){ //caso figlio
        int pid = getpid();
        int ppid = getppid();
        for(int j = 0; j < n_writes; j++){
            dprintf(fd, "pid = %d ppid = %d\n", pid, ppid);
        }
        exit(0); //spegne il processo del figlio
    }
    else { //caso padre
        while(wait(NULL) != -1);
    }
    close(fd);
    

}
