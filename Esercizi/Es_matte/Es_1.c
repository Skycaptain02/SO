#include <stdio.h>

int my_function();

int main(){
    int c = my_function();
    printf("%d", c);
    int v[10];
    //printf("%d",length(v));
    //int d[-1];
    printf("%ld", sizeof(v)/sizeof(v[0]));
}

int my_function(){
    return 5;
}


