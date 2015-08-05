#include <stdio.h>

int
main(void) {
	int n;
	while (scanf("%d", &n) != EOF && n > 0) {
		int sum = 0;
		int i = 0;
		for (;;) {
			if (i > n) {
				break;
			}

			sum += i;

			i++;
		}
		printf("sum %d\n", sum);
	}
	return 0;
}
