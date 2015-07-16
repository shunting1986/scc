#include <stdio.h>

int
main() {
	int i, j;
	int n, arr[1000]; // test local array

	while (scanf("%d", &n) != -1 && n > 0) {
		for (i = 0; i < n; i++) {
			scanf("%d", &arr[i]);
		}

		// do sort (can not handle passing array parameter yet)
		for (i = n - 1; i >= 1; i--) {
			for (j = 0; j < i; j++) {
				if (arr[j] > arr[j + 1]) {
					int t = arr[j];
					arr[j] = arr[j + 1];
					arr[j + 1] = t;
				}
			}
		}

		// print
		for (i = 0; i < n; i++) {
			printf("%d", arr[i]);
			if (i == n - 1) {
				printf("\n");
			} else {
				printf(" ");
			}
		}
	}
	return 0;
}
