#include "../src/env_var.h"
#include "list.h"

int main(){
    node * first = NULL;
    merci merce;
    merci * arr;
    int i;

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

    list_print(first, getpid());

    arr = list_to_array(first);

    printf("Before delete 0 and 3\n");

    for(i = 0; i < list_length(first); i++){
        printf("Type: %d\n", arr[i].type);
    }

    arr[0].type = -1;
    arr[3].type = -1;

    printf("After delete 0 and 3\n");

    for(i = 0; i < list_length(first); i++){
        printf("Type: %d\n", arr[i].type);
    }

    printf("length: %d\n", list_length(first));

    first = array_to_list(arr, list_length(first));
    list_print(first, getpid());
    printf("length after: %d\n", list_length(first));
}