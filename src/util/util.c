#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include <inc/util.h>

void 
// _panic(const char *fname, int line, char *fmt, ...) {
_panic(const char *fname, int line, char *fmt, ...) {
	va_list va;
	printf("%s:%d: ", fname, line);
	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
	putchar('\n');
	// assert(0 && "_panic");
	exit(1);
}
