#include <stdio.h>

struct st_type {
	int a, b, c, d, e;
};

union un_type {
	struct st_type st;
	int v;
};

int
main(void) {
	union un_type item = {
		.st.a = 3,
		.st.b = 5,
	};

	printf("st a %d\n", item.st.a);
	printf("st b %d\n", item.st.b);

	struct st_type stv = {
		.c = 5,
		.a = 7,
		8,
	};
	printf("stv a %d\n", stv.a);
	printf("stv b %d\n", stv.b);
	printf("stv c %d\n", stv.c);
	printf("stv d %d\n", stv.d);
	printf("stv e %d\n", stv.e);
	return 0;
}
