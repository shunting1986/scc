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

#define color(ctag, fmt) do { \
	va_list va; \
	va_start(va, fmt); \
	fprintf(stderr, "\033[" ctag "m"); \
	vfprintf(stderr, fmt, va); \
	fprintf(stderr, "\033[0m\n"); \
} while (0)

void red(const char *fmt, ...) {
	color("31", fmt);
}

// NOTE: the caller should free the memory
char *getdir(const char *path) {
	char *pos = strrchr(path, '/');
	if (pos == NULL || pos == path) {
		return strdup(".");
	}
	char *ret = mallocz(pos - path + 1);
	strncpy(ret, path, pos - path);
	return ret;
}


