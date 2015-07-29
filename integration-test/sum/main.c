#include <stdio.h>
/*
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdarg.h> 
 */

#ifndef EOF
#define EOF -1
#endif

int
main() {
	int n, i, sum;
	while (scanf("%d", &n) != EOF) { 
		sum = 0;
		for (i = 0; i <= n; i++) {
			sum += i;
		}
		printf("%d\n", sum);
	}
	return 0;
}
