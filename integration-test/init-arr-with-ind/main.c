#include <stdio.h>

int
main(void) {
	int arr[] = {
		[3 + 1] = 10,
		[20] = 3,
		[8] = 7,
		9,
	};
	int i;
	for (i = 0; i < sizeof(arr) / sizeof(*arr); i++) {
		printf("%d %d\n", i, arr[i]);
	}
	return 0;
}
