#include <stdio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int
main() {
	int a, b, c;
	
	while (scanf("%d%d%d", &a, &b, &c) != -1) {
		printf("%d\n", MIN(MAX(a, b), c));
		printf("%d\n", MAX(a, MIN(b, c)));
	}
	return 0;
}
