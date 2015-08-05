#include <stdio.h>

int 
main(void) {
	int a;
	long long b;
	while (scanf("%d%lld", &a, &b) != EOF) {
		printf("%.6f\n", (double) a / b * 2.0);
	}
	return 0;
}
