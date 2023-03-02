#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){
    /*int *p1, j;
    j = 50;
    p1 = &j;
    *p1 += 1;
    printf("%d, %d \n", j, *p1);

    int **p2;
    p2 = &p1; // int* = *(*)
    printf("%d", **p2);

    void *i[10];
    int a[10];
    int b[15];
    i[0] = a;
    i[1] = b;
    a[0] = 12;
    b[0] = 13;
    printf("%d", *(int *)i[1]);*/

    int v[10] = {1,9,1000}, *q = v+3;
    //q = v+1;
    //q++;
    //*q = *(v+1);
    //*q = *v+1;
    //q[4] = *(v+2);
    //v[1] = (int)*((char*)q-3);
    //q[-1] = *(((int *)&q)-9);
    //v[-1] = *(--q);
    for(int i = -1; i < 10; i++){
        printf("%d \n", v[i]);
    }
    
}