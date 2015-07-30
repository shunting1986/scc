#include <stdio.h>

#define glue(a, b) a ## b

int
main() {
	int xy = 3;
	int rs = 5;
	int ab = 4;
	printf("xy %d\n", glue(x, y));
	printf("rs %d\n", glue(r, s));
	printf("ab %d\n", glue(a, b));
	return 0;
}
