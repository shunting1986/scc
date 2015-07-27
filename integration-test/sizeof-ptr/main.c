#include <stdio.h>

typedef int mysize_t;

int
main() {
	char arr[15 * sizeof (int) - 4 * sizeof (void *) - sizeof (mysize_t)];
	printf("sizeof arr %d\n", sizeof(arr));
	return 0;
}
