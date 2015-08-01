#include <stdio.h>
#include <stdarg.h>

void msg(const char *fmt, ...) {	
	va_list va;
	va_start(va, fmt);
	vprintf(fmt, va);
	va_end(va);
}

int
main(void) {
	msg("msg %d, %s\n", 3, "hello");
	return 0;
}
