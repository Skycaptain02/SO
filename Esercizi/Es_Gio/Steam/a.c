#include <stdio.h>
#define _IONBUF 1


int main(int argc, char * argv[]){
    #if 0
        setvbuf(stdout, NULL, _IONBUF, 0);
    #endif
        setvbuf(stdout, NULL, _IONBUF, 0);
    #if 1
        fprintf(stdout, "Prima\n");
    #else
        fprintf(stdout,"Prima");
    #endif
        dprintf(1, "Dopo\n");
}