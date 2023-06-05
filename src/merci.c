#include "env_var.h"

int SO_PORTI, SO_NAVI, SO_MERCI, SO_SIZE, SO_MIN_VITA, SO_MAX_VITA, SO_LATO, SO_SPEED, SO_CAPACITY, SO_BANCHINE; 
int SO_FILL, SO_LOADSPEED, SO_DAYS, SO_STORM_DURATION, SO_SWELL_DURATION, SO_MAELESTROM, PRINT_MERCI, CONVERSION_SEC_NSEN;

void readInputs();

int main(int argc, char * argv[]){
    int i, shm_id;
    Merce * tipi_merci;

    readInputs();
    srand(getpid());

    shm_id = shmget(getppid() + 1, sizeof(Merce) * SO_MERCI, 0600 | IPC_CREAT);
    tipi_merci = shmat(shm_id, NULL, 0);
    
    for(i = 0; i < SO_MERCI; i++){
        tipi_merci[i].type = (i+1);
        tipi_merci[i].weight = ((rand() % SO_SIZE) + 1);
        tipi_merci[i].life = ((rand() % SO_MAX_VITA) + SO_MIN_VITA);
    }

    if(PRINT_MERCI){
        for(i = 0; i < SO_MERCI; i++){
            printf("Tipo -> %d, Peso -> %d, Vita -> %d\n", tipi_merci[i].type, tipi_merci[i].weight, tipi_merci[i].life);
        }
    }


}

void readInputs(){
    const char * file_path = "../src/env_var.txt";
    char buffer[1024];
    char buffer_cpy[1024];
    ssize_t bytes_read;
    char * token;
    FILE * file_descriptor = fopen(file_path, "r");
    

    if (!file_descriptor) {
        perror("Failed to open the file");
    }

    while (fgets(buffer, 1024, file_descriptor)){
        token = strtok(buffer, " ");
        if(strcmp(token, "SO_NAVI") == 0){
            token = strtok(NULL, " ");
            SO_NAVI = atoi(token);
        }else if(strcmp(token, "SO_PORTI") == 0){
            token = strtok(NULL, " ");
            SO_PORTI = atoi(token);
        }else if(strcmp(token, "SO_MERCI") == 0){
            token = strtok(NULL, " ");
            SO_MERCI = atoi(token);
        }else if(strcmp(token, "SO_SIZE") == 0){
            token = strtok(NULL, " ");
            SO_SIZE = atoi(token);
        }else if(strcmp(token, "SO_MIN_VITA") == 0){
            token = strtok(NULL, " ");
            SO_MIN_VITA = atoi(token);
        }else if(strcmp(token, "SO_MAX_VITA") == 0){
            token = strtok(NULL, " ");
            SO_MAX_VITA = atoi(token);
        }else if(strcmp(token, "SO_LATO") == 0){
            token = strtok(NULL, " ");
            SO_LATO = atoi(token);
        }else if(strcmp(token, "SO_SPEED") == 0){
            token = strtok(NULL, " ");
            SO_SPEED = atoi(token);
        }else if(strcmp(token, "SO_CAPACITY") == 0){
            token = strtok(NULL, " ");
            SO_CAPACITY = atoi(token);
        }else if(strcmp(token, "SO_BANCHINE") == 0){
            token = strtok(NULL, " ");
            SO_BANCHINE = atoi(token);
        }else if(strcmp(token, "SO_FILL") == 0){
            token = strtok(NULL, " ");
            SO_FILL = atoi(token);
        }else if(strcmp(token, "SO_LOADSPEED") == 0){
            token = strtok(NULL, " ");
            SO_LOADSPEED = atoi(token);
        }else if(strcmp(token, "SO_DAYS") == 0){
            token = strtok(NULL, " ");
            SO_DAYS = atoi(token);
        }else if(strcmp(token, "SO_STORM_DURATION") == 0){
            token = strtok(NULL, " ");
            SO_STORM_DURATION = atoi(token);
        }else if(strcmp(token, "SO_SWELL_DURATION") == 0){
            token = strtok(NULL, " ");
            SO_SWELL_DURATION = atoi(token);
        }else if(strcmp(token, "SO_MAELESTROM") == 0){
            token = strtok(NULL, " ");
            SO_MAELESTROM = atoi(token);
        }else if(strcmp(token, "PRINT_MERCI") == 0){
            token = strtok(NULL, " ");
            PRINT_MERCI = atoi(token);
        }else if(strcmp(token, "CONVERSION_SEC_NSEN") == 0){
            token = strtok(NULL, " ");
            CONVERSION_SEC_NSEN = atoi(token);
        }
    }
    fclose(file_descriptor);
}