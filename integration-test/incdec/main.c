#include <stdio.h>

short int a = 3;
short int b = 50;

int
main(void) {
	printf("a++ %d\n", a++);
	printf("++a %d\n", ++a);
	printf("a %d\n", a);
	printf("b %d\n", b);
	return 0;
}
