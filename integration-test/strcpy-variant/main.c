#include <stdio.h>

char buf[5];

int
main(void) {
	int v;
	buf[0] = 'a';
	buf[1] = 'b';
	buf[2] = 'c';
	buf[3] = 'd';
	buf[0] = 'e';
	printf("buf is %s\n", buf);
	v = buf[0] == 'e';
	printf("check buf 0 %d\n", v);
	return 0;
}
