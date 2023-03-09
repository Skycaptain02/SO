#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>


int main(int argc, char * argv[]){
    char * file_name;
    int n_kids;
    int n_writes;

    file_name = argv[1];
    n_kids = strtol(argv[2], NULL, 10);
    n_writes = strtol(argv[3], NULL, 10);

    printf("Nome: %s, n_kids: %d, n_writes: %d\n", file_name, n_kids, n_writes);
    
    int fd;
    fd = open(file_name, O_WRONLY);

    int i;
    int id;
    for(i = 0; i < n_kids; i++){
        id = fork();
        
        if(id){
            break;
        }
        
    }

    if(id != 0){//CASO FIGLIO
        int j = 0;
        int pid;
        int ppid;
        pid  = getpid();
        ppid = getppid();
        for(j = 0; j < n_writes; j++){
            dprintf(fd, "pid: %d; ppid: %d;\n", pid, ppid);
        }
        printf("Ended printing\n");
        exit(0);
    }
    else{//CASO PADRE
        while(wait(NULL) != -1);
    }


    close(fd);

}