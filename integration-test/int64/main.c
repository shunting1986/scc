#include <stdio.h>

long long add(long long a, long long b) {
	return a + b;
}

int
main(void) {
	long long a, b;
	while (scanf("%lld%lld", &a, &b) != EOF) {
		printf("%lld %d %d\n", add(a, b), a < b, a > b);
		printf("eq %d, ne %d\n", a == b, a != b);
		a += b;
		printf("after += a %lld\n", a);
	}
	return 0;
}
