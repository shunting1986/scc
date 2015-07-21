#include <stdio.h>

int check(int v) {
	printf("check %d\n", v);
	return v;
}

int
main() {
	int a, b, c, d;
	while (scanf("%d%d%d%d", &a, &b, &c, &d) != -1) {
		printf("%d\n", a > b ? check(c) : check(d));	
	}
	return 0;
}
