#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include <inc/util.h>

void 
// _panic(const char *fname, int line, char *fmt, ...) {
_panic(const char *fname, int line, const char *funcname, const char *fmt, ...) {
	va_list va;
	fprintf(stderr, "%s:%d: %s ", fname, line, funcname);
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fprintf(stderr, "\n");
	// assert(0 && "_panic");
	exit(1);
}
