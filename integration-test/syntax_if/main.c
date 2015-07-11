#include <stdio.h>

int
main() {
	int v;
	while (scanf("%d", &v) != -1) {
		if (v >= 10) {
			printf("%d\n", v);
		}
	}
	return 0;
}
