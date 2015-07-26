#include <stdio.h>

enum enum_type {
	zero,
	two = 2,
	three,
	ten = 10,
};

int
main() {
	printf("%d\n", zero);
	printf("%d\n", two);
	printf("%d\n", three);
	printf("%d\n", ten);
	return 0;
}
