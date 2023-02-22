#include <stdio.h>

int my_function();

int main(){
    int c = my_function();
    printf("%d", c);
}

int my_function(){
    return 5;
}


