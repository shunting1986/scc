#include <stdio.h>

struct st_type {
	int a;
	char *b;
	short c;
	char d;
} first;

int
main(void) {
	struct st_type second;
	second.a = 3;
	second.b = "hello";
	second.c = 5;
	second.d = 'v';

	first = second;
	printf("%d %s %d %c\n", first.a, first.b, first.c, first.d);
	return 0;
}
