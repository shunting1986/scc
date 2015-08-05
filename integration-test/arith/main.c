#include <stdio.h>

int
main(void) {
	int v;
	while (scanf("%d", &v) != EOF) {
		int m = v;
		int d = v;
		int md = v;
		int orv = v;
		m *= 2;
		printf("*=2 %d\n", m);
		d /= 3;
		printf("/=3 %d\n", d);
		md %= 10;
		printf("%%=10 %d\n", md);
		orv |= 7;
		printf("|=7 %d\n", orv);

		printf("!v %d\n", !v);
	}
	return 0;
}
