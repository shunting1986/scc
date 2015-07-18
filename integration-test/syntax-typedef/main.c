#include <stdio.h>

typedef int A[10][10];

int
main() {
	A x;
	printf("sizeof(A) = %d\n", sizeof(A));
	{
		typedef int A;
		A y;

		printf("sizeof(A) = %d\n", sizeof(A));
		printf("sizeof(x) = %d\n", sizeof(x));
		printf("sizeof(y) = %d\n", sizeof(y));

		y = 8;
		printf("sizeof(y++) %d\n", sizeof(++y)); 
		printf("y = %d\n", y); // NOTE: gcc report 8 here
	}
	return 0;	
}
