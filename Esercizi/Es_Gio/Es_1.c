#include <stdio.h>

int main(){
    char a[] = "Ciao";
    char b[] = " Dopo";

    int x = sizeof(a)/sizeof(a[0]);
    int y = sizeof(b)/sizeof(b[0]);
    int totSize = x + y;

    char str[totSize];
    int k = 0;
    for(int i = 0; i < x-1; i++){
        str[k] = a[i];
        k++;
    }
    for(int j = 0; j < y; j++){
        str[k] = b[j];
        k++;
    }

    printf("%s \n", str);
}