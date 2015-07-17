#include <stdio.h>
#include <stdlib.h>

int a;
int n;
int *p1 = &a;

int
main() {
	int *p2;
	int i;
	while (scanf("%d%d", &n, p1) != -1) {
		p2 = malloc(n * 4);
		for (i = 0; i < n; i++) {
			scanf("%d", &p2[i]);
		}
		for (i = 0; i < n; i++) {
			printf("%d", p2[i] * *p1);
			if (i == n - 1) {
				printf("\n");
			} else {
				printf(" ");
			}
		}
		free(p2);
	}
	return 0;
}
