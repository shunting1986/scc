#include <stdio.h>

#define s "Hello"
#define calc a + b * c - d

int main() {
	int a = 2, b = 3, c = 5, d = 8;
	printf("msg: %s\n", s);
	printf("calc: %d\n", calc);
	return 0;
}
