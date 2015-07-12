#include <stdio.h>

int check(int v) {
	printf("check %d\n", v);
	return v > 0;
}

int
main() {
	int a, b, c;
	while (scanf("%d%d%d", &a, &b, &c) != -1) {
		if (check(a) != 0 && check(b) != 0 && check(c) != 0) {
			printf("true\n");
		} else {
			printf("false\n");
		}

		if (check(a) != 0 || check(b) != 0 || check(c) != 0) {
			printf("true\n");
		} else {
			printf("false\n");
		}
	}
	return 0;
}
