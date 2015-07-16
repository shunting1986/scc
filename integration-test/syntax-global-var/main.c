#include <stdio.h>

int a;

void inc() {
	a++;
	return;
}

int
main() {
	while (scanf("%d", &a) != -1) {
		inc();
		printf("%d\n", a);
	}
	return 0;
}
