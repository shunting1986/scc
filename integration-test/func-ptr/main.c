#include <stdio.h>

int add(int a, int b) {
	return a + b;
}

int sub(int a, int b) {
	return a - b;
}

int mul(int a, int b) {
	return a * b;
}

int div(int a, int b) {
	return a / b;
}

typedef int (*func_type)(int a, int b);
func_type fplist[] = {
	add,
	sub,
	mul,
	div,
};

int doop(func_type, int a, int b);

int doop(func_type op, int a, int b) {
	return op(a, b);
}

int
main(void) {
	int i, a, b;
	while (scanf("%d%d", &a, &b) != EOF) {
		for (i = 0; i < sizeof(fplist) / sizeof(*fplist); i++) {
			printf("%d ", doop(fplist[i], a, b));
		}
		printf("\n");
	}
	return 0;
}
