#include <stdio.h>

int check(int v) {
	printf("check %d\n", v);
	return v;
}

int
main() {
	int a, b, c, d, e;
	while (scanf("%d%d%d%d%d", &a, &b, &c, &d, &e) != -1) {
		printf("%d\n", a > 0 ? check(b) : check(c) < 0 ? check(d) : check(e));	
	}
	return 0;
}
