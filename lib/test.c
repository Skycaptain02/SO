#include "list.h"
#include "../src/env_var.h"
List lista;

void inserisci(Merce merce);

int main(){
    int i;
    
    Merce merce_1, merce_2, merce_3, merce_4, merce_5, merce_6;
    Merce * arr;
    int * merce_scaduta;
    merce_1.type = 0;
    merce_1.life = -1;
    merce_1.weight = 0;

    merce_2.type = 1;
    merce_2.life = 1;
    merce_2.weight = 0;

    merce_3.type = 2;
    merce_3.life = 1;
    merce_3.weight = 5;

    merce_4.type = 2;
    merce_4.life = 1;
    merce_4.weight = 5;

    merce_5.type = 2;
    merce_5.life = 0;
    merce_5.weight = 5;

    merce_6.type = 2;
    merce_6.life = 3;
    merce_6.weight = 5;


    
    arr = malloc(sizeof(Merce) * 6);
    merce_scaduta = malloc(sizeof(int));

    arr[0] = merce_1;
    arr[1] = merce_2;
    arr[2] = merce_3;
    arr[3] = merce_4;
    arr[4] = merce_5;
    arr[5] = merce_6;

    listCreate(&lista);
    listInsert(&lista, arr[0]);
    listInsert(&lista, arr[1]);
    listInsert(&lista, arr[2]);
    listInsert(&lista, arr[3]);
    listInsert(&lista, arr[4]);
    listInsert(&lista, arr[5]);
    listPrint(&lista);
    listFree(&lista);
    listPrint(&lista);
}