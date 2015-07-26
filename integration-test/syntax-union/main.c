#include <stdio.h>

#ifndef EOF
#define EOF -1
#endif

union u_a {
	int a;
	int b;
};

int
main() {
	int v;
	union u_a u;
	int i;
	while (scanf("%d", &v) != EOF) {
		u.a = v;	
		printf("%d\n", u.b);
	}
	return 0;
}
