#include <stdio.h>

long long add(long long a, long long b) {
	return a + b;
}

int
main(void) {
	// XXX: this test case does not really check value beyond 32 bits yet, we just make sure
	// the values within 32 bit range are handled correctly
	long long a, b;
	while (scanf("%lld%lld", &a, &b) != EOF) {
		printf("%lld %d\n", add(a, b), a < b);
	}
	return 0;
}
