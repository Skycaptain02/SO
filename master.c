#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include "env_var.h"

#define NOME_NAVI "./navi.c"
#define NOME_PORTI
#define NOME_METEO

int main(){
    int i;
    pid_t * pid_navi;
    char* args_navi[] = {NOME_NAVI, NULL};
    char* args_porti[] = {NOME_NAVI, NULL};
    pid_navi = malloc(sizeof(pid_navi) * SO_NAVI);

    

    for(i = 0; i < SO_NAVI; i++){
        switch (pid_navi[i] = fork())
        {
        case 0: /*Caso nave*/
                execve("navi.c", args_navi , NULL);
            break;
        
        default:
            break;
        }
    }
}