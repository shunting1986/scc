#include <stdio.h>

int n;
int A[100][100];

int 
main() {
	int B[100][100];
	int i, j, k, sum;

	while (scanf("%d", &n) != -1 && n > 0) {
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) {
				scanf("%d", &A[i][j]);
			}
		}
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) {
				scanf("%d", &B[i][j]);
			}
		}

		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) {
				sum = 0;
				for (k = 0; k < n; k++) {
					sum += A[i][k] * B[k][j];
				}
				printf("%d", sum);
				if (j == n - 1) {
					printf("\n");
				} else {
					printf(" ");
				}
			}
		}
		printf("\n");
	}
	return 0;
}
