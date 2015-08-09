#include <stdio.h>

struct st_type {
	int a, b;
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
	return 0;
}
