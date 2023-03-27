#include "env_var.h"

int create_navi = 0;
int create_porti = 0;

void handler_navi_porti(int signal){
    switch(signal){
        case SIGUSR1:
            create_navi++;
            printf("Ho incrementato il valore\n");
        break;
        case SIGUSR2:
            create_porti++;
        break;
    }
}

int main(int argc, char * argv[]){
    
    struct sigaction sa;

    int i, status, errno;
    pid_t * pid_navi, * pid_porti, pid_meteo;
    char* args_navi[] = {"./navi", NULL};
    char* args_porti[] = {"./porti", NULL};
    char* args_meteo[] = {"./meteo", NULL};
    pid_navi = malloc(sizeof(pid_navi) * SO_NAVI);
    pid_porti = malloc(sizeof(pid_porti) * SO_PORTI);

    bzero(&sa, sizeof(sa));
    sa.sa_handler = handler_navi_porti;
    sigaction(SIGUSR1, &sa, NULL);




    for(i = 0; i < SO_NAVI; i++){
        switch (pid_navi[i] = fork())
        {
            case -1:
                    printf("C'è stato un errore nel fork per le navi: %s", strerror(errno));
                    exit(-1);
                break;
            case 0:
                execve("./navi", args_navi , NULL);
            break;
            default:
            break;
        }
    }

    /*
    for(i = 0; i < SO_PORTI; i++){
        switch (pid_porti[i] = fork())
        {
            case -1:
                    printf("C'è stato un errore nel fork per i porti: %s", strerror(errno));
                    exit(-1);
                break;
            case 0:
            default:
            break;
        }

    }
    */

    while(1){
        printf("%d\n", create_navi);
        if(create_navi == SO_NAVI){
            break;
        }
    }
    printf("SONO USCITO\n");
}
