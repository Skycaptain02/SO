#include <stdio.h>

int myfunction(){
    return 5;
}

int main(){
    int c = 0;
    c = myfunction();
    printf("%d", c);
}