#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){
    int s[7];
    for(int i = 0; i < 7; i++){
        printf("Inserisci \n");
        scanf("%i \n", &s[i]);
    }
    for(int i = 0; i < 7; i++ ){
        printf("%d \n",s[i]);
    }
}