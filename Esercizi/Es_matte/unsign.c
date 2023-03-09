#include <stdio.h>

int main(){
	unsigned int a = -1;
	int b = 1;
	int c = a + b;
	a = b;
	printf("%i", c);
}
