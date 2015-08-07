#include <stdio.h>

struct st_type {
	int a;
	int arr[];
};

int
main(void) {
	printf("size %d\n", sizeof(struct st_type));
	return 0;
}
