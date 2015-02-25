#include <stdlib.h>
#include <stdio.h>

#include "util.h"

void 
panic(char *str) {
	printf("%s\n", str);
	exit(1);
}
