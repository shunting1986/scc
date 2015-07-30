#include <stdio.h>

int
main(void) {
#undef A
#undef B
#undef C
#undef D
#undef E
#undef msg
#define A 1
#define B 2
#define C -1
#define D -4
#define E -5
#if (A > 0 ? B : C < 0 ? D : E) > 0
#define msg "true"
#else
#define msg "false"
#endif
	printf("msg is %s\n", msg);

#undef A
#undef B
#undef C
#undef D
#undef E
#undef msg
#define A 1
#define B 5
#define C 8
#define D 9
#define E 7
#if (A > 0 ? B : C < 0 ? D : E) > 0
#define msg "true"
#else
#define msg "false"
#endif
	printf("msg is %s\n", msg);

#undef A
#undef B
#undef C
#undef D
#undef E
#undef msg
#define A -1
#define B 8
#define C 3
#define D 9
#define E 2
#if (A > 0 ? B : C < 0 ? D : E) > 0
#define msg "true"
#else
#define msg "false"
#endif
	printf("msg is %s\n", msg);

#undef A
#undef B
#undef C
#undef D
#undef E
#undef msg
#define A -1
#define B 7
#define C -3
#define D -1
#define E 8
#if (A > 0 ? B : C < 0 ? D : E) > 0
#define msg "true"
#else
#define msg "false"
#endif
	printf("msg is %s\n", msg);

	return 0;
}
