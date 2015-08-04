#include <stdio.h>

struct st_type {
	int a, b;
};

int
main(void) {
	struct st_type ar[5];		
	printf("diff %d\n", &ar[3] - &ar[1]);
	return 0;
}
