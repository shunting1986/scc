#include <stdio.h>

void func(int v) {
	if (v == 1) {
		goto handle1;
	} else if (v == 2) {
		goto handle2;
	} else {
		goto handle_others;
	}
handle1:
	printf("func get 1\n");
	return;
handle2:
	printf("func get 2\n");
	return;
handle_others:
	printf("func get others\n");
}

int
main(void) {
	int v;
	while (scanf("%d", &v) != EOF) {
		func(v);
		if (v == 1) {
			goto handle1;
		} else if (v == 2) {
			goto handle2;
		} else {
			goto handle_others;
		}
handle1:
		printf("get 1\n");
		continue;
handle2:
		printf("get 2\n");
		continue;
handle_others:
		printf("get others\n");
		continue;
	}
	return 0;
}
