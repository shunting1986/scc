#include <stdio.h>

struct st_type {
	int a;
	union {
		int b, c;
	};
	int d;
};

struct st_type st_item = {
	3,
	{ 4},
	6
};

int
main(void) {
	printf("a %d\n", st_item.a);
	printf("b %d\n", st_item.b);
	printf("c %d\n", st_item.c);
	printf("d %d\n", st_item.d);
	return 0;
}
