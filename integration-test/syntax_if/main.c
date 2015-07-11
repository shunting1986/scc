#include <stdio.h>

// return less, greate, equals according to the value of the 2 numbers
int main(void) {
	int a, b;
	while (scanf("%d%d", &a, &b) != -1) {
		if (a > b) {
			printf("greater\n");
		} else if (a < b) {
			printf("less\n");
		} else {
			printf("equal\n");
		}
	}
	return 0;
}
