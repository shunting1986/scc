#include <stdio.h>

#undef EOF
#define EOF -1

int
main() {
	int n, i, sum;
	while (scanf("%d", &n) != EOF) { 
	// while (scanf("%d", &n) != -1) {
		sum = 0;
		for (i = 0; i <= n; i++) {
			sum += i;
		}
		printf("%d\n", sum);
	}
	return 0;
}
