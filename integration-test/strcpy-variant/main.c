#include <stdio.h>

char buf[5];

int
main(void) {
	buf[0] = 'a';
	buf[1] = 'b';
	buf[2] = 'c';
	buf[3] = 'd';
	buf[0] = 'e';
	printf("buf is %s\n", buf);
	return 0;
}
