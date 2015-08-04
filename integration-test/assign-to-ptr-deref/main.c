#include <stdio.h>

int
main(void) {
	int v, t;
	int *p = &t;
	while (scanf("%d", &v) != EOF) {
		*p = v;
		*p += v;
		printf("%d\n", t);
	}
	return 0;
}
