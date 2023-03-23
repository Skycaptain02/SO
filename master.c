#include "env_var.h"

int main(){
    int i = 0, status, errno;
    pid_t * pid_navi, * pid_porti, pid_meteo;
    char* args_navi[] = {"./navi", NULL};
    char* args_porti[] = {"./porti", NULL};
    char* args_meteo[] = {"./meteo", NULL};
    pid_navi = malloc(sizeof(pid_navi) * SO_NAVI);
    pid_porti = malloc(sizeof(pid_porti) * SO_PORTI);

    for(i = 0; i < SO_NAVI; i++){
        switch (pid_navi[i] = fork())
        {
            case -1:
                    printf("C'è stato un errore nel fork per le navi: %d", strerror(errno));
                    exit(-1);
                break;
            case 0: /*Caso nave*/
                    execve("./navi", args_navi , NULL);
                    exit(0); /*Non dovrebbe servire*/
                break;
            
            default:
                /*printf("Creato processo nave con [PID]: %d, i: %d\n", pid_navi[i], i);*/
                break;
        }
    }

    for(i = 0; i < SO_PORTI; i++){
        switch (pid_porti[i] = fork())
        {
            case -1:
                    printf("C'è stato un errore nel fork per i porti: %d", strerror(errno));
                    exit(-1);
                break;
            case 0: /*Caso nave*/
                    execve("./porti", args_porti , NULL);
                    exit(0); /*Non dovrebbe servire*/
                break;
            
            default:
                /*printf("Creato processo porto con [PID]: %d, i: %d\n", pid_porti[i], i);*/ 
                break;
        }
    }

    switch(pid_meteo = fork()){
        case -1:
                printf("C'è stato un errore nel fork per il meteo: %d", strerror(errno));
                exit(-1);
            break;
            
        case 0:
                execve("./meteo", args_meteo , NULL);
                exit(0); /*Non dovrebbe servire*/
            break;

        default:
            /*printf("Creato processo meteo con [PID]: %d\n", pid_meteo);*/
            break;
    }

    while(wait(&status) != -1);
}