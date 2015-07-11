#include <stdio.h>

int
main() {
	int a, b;
	while (scanf("%d%d", &a, &b) != -1) {
		if (a > 0 && b > 0) {
			printf("1");
		} else {
			printf("0");
		}
		printf("\n");
	}
	return 0;
}
