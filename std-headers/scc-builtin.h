#define scc_align_down(v, a) ((v) / (a) * (a))
#define scc_align_up(v, a) scc_align_down(v + a - 1, a)

#if 1
#define __builtin_va_list void *
#define __builtin_va_end(ap)
#define __builtin_va_start(ap, fmt) (ap = &fmt, ap = ap + 4, ap)
#define __builtin_va_arg(ap, type) (ap += scc_align_up(sizeof(type), 4), (type) (ap - scc_align_up(sizeof(type), 4)))
#endif

#define __need_IOV_MAX
#include <stdint.h>

#undef INT64_MAX
// XXX the way we parse pp number can not handle the standard definition of 
// INT64_MAX right now
// #define INT64_MAX ((1LL << 63) - 1)
#define INT64_MAX 0x7FFFFFFFFFFFFFFFLL

extern int printf(const char *__format, ...);
extern void exit(int __status);

static void scc_builtin_abort() {
	printf("scc does not support this feature yet\n");
	exit(1);
}

typedef unsigned char _Bool;
