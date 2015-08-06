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
main(void) {
	int n, i, sum;
	while (scanf("%d", &n) != EOF) { 
		sum = 0;

#if 0
		for (i = 0; i <= n; i++) {
			sum += i;
		}
#else
		i = 0;
		while (1) {
			if (i > n) {
				break;
			}
			sum += i;
			i++;
		}
#endif
		printf("%d\n", sum);
	}
	return 0;
}
