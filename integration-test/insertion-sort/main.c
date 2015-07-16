#include <stdio.h>

int arr[1000];
int n;

void sort() {
	int i, j;
	for (i = 1; i < n; i++) {
		int v; // purposely put the initializer in the following statement since we do not support intializer yet
		v = arr[i];
		for (j = i - 1; j >= 0 && arr[j] > v; j--) {
			arr[j + 1] = arr[j];
		}
		arr[j + 1] = v;
	}
	return; // XXX need auto inserting return
}

int
main() {
	int i;
	// while (scanf("%d", &n) != EOF && n > 0) {
	while (scanf("%d", &n) != -1 && n > 0) {
		for (i = 0; i < n; i++) {
			scanf("%d", &arr[i]);
		}
		sort();
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
