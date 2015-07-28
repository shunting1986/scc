#include <stdio.h>

#ifndef EOF
#define EOF -1
#endif

int
main() {
	int n, i, sum;
	while (scanf("%d", &n) != EOF) { 
		sum = 0;
		for (i = 0; i <= n; i++) {
			sum += i;
		}
		printf("%d\n", sum);
	}
	return 0;
}
