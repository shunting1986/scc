#include <stdio.h>

int 
main(void) {
	int a, b;
	while (scanf("%d%d", &a, &b) != EOF) {
		printf("%.6f\n", (double) a / b);
	}
	return 0;
}
