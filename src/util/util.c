#include <stdlib.h>
#include <stdio.h>

#include <inc/util.h>

void 
panic(char *str) {
	printf("%s\n", str);
	exit(1);
}
