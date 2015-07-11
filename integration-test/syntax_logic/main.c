#include <stdio.h>

int check(int v) {
	printf("check %d\n", v);
	return v > 0;
}

int
main() {
	int a, b;
	while (scanf("%d%d", &a, &b) != -1) {
		if (check(a) != 0 && check(b) != 0) {
			printf("true\n");
		} else {
			printf("false\n");
		}
	}
	return 0;
}
