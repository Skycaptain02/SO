#include <stdio.h>

int my_function();

int main(){
    int c = my_function();
    printf("%d", c);
    //printf("%d",length(v));
    //int d[-1];
    printf("%ld", sizeof(v)/sizeof(v[0]));
    char v_2[] = "Ciao";
    int v[10] = {3, -1, 4};
    for(int i = 0; i < 10; i++){
        printf("%d",v[i]);
    }
}

int my_function(){
    return 5;
}


