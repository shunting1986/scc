#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <inc/util.h>

void 
panic(char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
	putchar('\n');
	exit(1);
}
