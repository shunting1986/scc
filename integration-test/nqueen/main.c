#include <stdio.h>

int n;
int state[1000];

int abs(int v) {
	if (v > 0) {
		return v;
	} else {
		return -v;
	}
}

int check(int step, int v) {
	int i;
	for (i = 0; i < step; i++) {
		if (v == state[i] || abs(step - i) == abs(v - state[i])) {
			return 0;
		}
	}
	return 1;
}

int search(int step) {
	int ret = 0, i;
	if (step == n) {
		return 1;
	}

	for (i = 0; i < n; i++) {
		if (check(step, i)) {
			state[step] = i;
			ret += search(step + 1);
		}
	}
	return ret;
}

int
main() {
	while (scanf("%d", &n) != -1 && n > 0) {
		printf("%d\n", search(0));	
	}
	return 0;
}
