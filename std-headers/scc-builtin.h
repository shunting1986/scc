#define __builtin_va_list void *
#define __builtin_va_end(ap)
#define __builtin_va_start(ap, fmt) (ap = &fmt, ap = ap + 4, ap)

extern int printf(const char *__format, ...);
extern void exit(int __status);

static void scc_builtin_abort() {
	printf("scc does not support this feature yet\n");
	exit(1);
}
