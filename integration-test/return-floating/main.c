#include <stdio.h>
#include <stdlib.h>

int
main(void) {
	char buf[256];
	while (scanf("%s", buf) != EOF) {
		double v = atof(buf);
		printf("%.6f\n", v + 1);
	}
	return 0;
}
