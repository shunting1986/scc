#include <stdio.h>
#include <stdlib.h>

int **A;

int **alloc(int n) {
	int **ret = malloc(n * 4);
	int i;
	for (i = 0; i < n; i++) {
		ret[i] = malloc(n * 4);
	}
	return ret;
}

void freearr(int **arr, int n) {
	int i;
	for (i = 0; i < n; i++) {
		free(arr[i]);
	}
	free(arr);
}

int 
main() {
	int **B;
	int i, j, k, sum, n;

	while (scanf("%d", &n) != -1 && n > 0) {
		A = alloc(n);
		B = alloc(n);
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
		freearr(A, n);
		freearr(B, n);
	}
	return 0;
}
