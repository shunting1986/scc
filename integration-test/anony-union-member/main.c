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

struct st_type2 {
	int a;
	struct {
		int b, c;
	};
	int d;
};

struct st_type2 st_item2 = {
	3,
	{ 4, 5},
	6
};


int
main(void) {
	printf("a %d\n", st_item.a);
	printf("b %d\n", st_item.b);
	printf("c %d\n", st_item.c);
	printf("d %d\n", st_item.d);

	printf("a %d\n", st_item2.a);
	printf("b %d\n", st_item2.b);
	printf("c %d\n", st_item2.c);
	printf("d %d\n", st_item2.d);
	return 0;
}
