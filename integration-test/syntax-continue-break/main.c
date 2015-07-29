#include <stdio.h>

int arr[1024];

int
main(void) {
	int n;
	while (scanf("%d", &n) != EOF) {
		int i;
		int sum = 0;
		for (i = 0; i < n; i++) {
			scanf("%d", &arr[i]);
		}
		
		// break for
		sum = 0;	
		for (i = 0; i < n; i++) {
			if (arr[i] < 0) {
				break;
			}
			sum += arr[i];
		}
		printf("break for %d\n", sum);

		// continue for
		sum = 0;
		for (i = 0; i < n; i++) {
			if (arr[i] < 0) {
				continue;
			}
			sum += arr[i];
		}
		printf("continue for %d\n", sum);

		// break while
		sum = 0;
		i = 0;
		while (i < n) {
			if (arr[i] < 0) {
				break;
			}
			sum += arr[i];
			i++;
		}
		printf("break while %d\n", sum);

		// continue while
		sum = 0;
		i = 0;
		while (i < n) {
			if (arr[i] < 0) {
				i++;
				continue;
			}
			sum += arr[i];
			i++;
		}
		printf("continue while %d\n", sum);

		// break do while
		sum = 0;
		i = 0;
		if (n > 0) {
			do {
				if (arr[i] < 0) {
					break;
				}
				sum += arr[i];
				i++;
			} while (i < n);
		}
		printf("break do-while %d\n", sum);

		// continue do while
		sum = 0;
		i = 0;
		if (n > 0) {
			do {
				if (arr[i] < 0) {
					i++;
					continue;
				}
				sum += arr[i];
				i++;
			} while (i < n);
		}
		printf("continue do-while %d\n", sum);
	}
	return 0;
}
