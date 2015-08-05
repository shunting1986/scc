#include <stdio.h>

short func(short v) {
	return v + 1;
}

int
main(void) {
	short v = 3;
	printf("%d\n", func(v));
	return 0;
}
