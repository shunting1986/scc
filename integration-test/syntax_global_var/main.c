#include <stdio.h>

int a;

void inc() {
	a++;
}

int
main(void) {
	while (scanf("%d", &a) != -1) {
		inc();
		printf("%d\n", a);
	}
	return 0;
}
