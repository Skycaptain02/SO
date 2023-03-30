#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
int main(){
    srand(getpid());

    int x = 20;
    int r;
    r = (rand() % (x+1)) - (x/2);

    for(int i = 0; i < 20; i++){
        printf("Numero: %d\n", r);
        r = (rand() % (x+1)) - (x/2);
    }

}