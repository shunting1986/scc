#include <stdio.h>

int
main(void) {
	int v;
	while (scanf("%d", &v) != EOF) {
		int m = v;
		int d = v;
		m *= 2;
		printf("*=2 %d\n", m);
		d /= 3;
		printf("/=3 %d\n", d);
	}
	return 0;
}
