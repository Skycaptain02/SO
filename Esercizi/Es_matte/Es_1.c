#include <stdio.h>
#include <string.h>

int main(){
    char s_1[] = "Ciao";
    char s_2[] = " Alberto";
    int n;
    n = ((sizeof(s_1)/sizeof(s_1[0])) + (sizeof(s_2)/sizeof(s_2[0])));
    char r[n];
    int i = 0;
    while(s_1[i] != 0){
        r[i] = s_1[i];
        i++;
    }
    int j = 0;
    while(s_2[j] != 0){
        r[i] = s_2[j];
        j++;
        i++;
        if(s_2[j] == 0){
            r[i] = 0;
        }
    }
    printf("%s", r);
}

