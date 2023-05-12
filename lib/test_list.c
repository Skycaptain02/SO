#include "../src/env_var.h"
#include "list.h"

int main(){
    node * first = NULL;
    merci merce;
    merci * arr;
    int i;
    int * life;

    life = malloc(sizeof(int));

    merce.type = 1;
    merce.life = 12;
    merce.weight = 14;

    first = list_insert(first,merce);

    merce.type = 2;
    merce.life = 10;
    merce.weight = 12;

    first = list_insert(first,merce);

    merce.type = 3;
    merce.life = 10;
    merce.weight = 12;

    first = list_insert(first,merce);

    merce.type = 4;
    merce.life = 10;
    merce.weight = 12;

    first = list_insert(first,merce);

    merce.type = 5;
    merce.life = 10;
    merce.weight = 12;

    first = list_insert(first,merce);

    list_print(first);

    first = list_remove_elem(first, 1, life);

    printf("Vita : %d\n", *life);
    
    list_print(first);
}