#include <stdio.h>

int
main(void) {
	int v;
	while (scanf("%d", &v) != EOF) {
		printf("switch 1\n");
		switch (v) {
		case 1:
			printf("one\n");
			break;
		case 2:
			printf("two\n");
			break;
		case 3:
		case 4:
			printf("three or four\n");
			break;
		}
		printf("switch 2\n");
		switch (v) {
		case 1:
			printf("one\n");
			break;
		default:
			printf("more\n");
			break;
		case 2:
			printf("two\n");
			break;
		}

		printf("switch 3\n");
		switch (v) { 
		default:
			printf("catch all\n");
			break;
		}

		printf("switch 4\n");
		switch (v) {
			// empty body
		}
	}
	return 0;
}
