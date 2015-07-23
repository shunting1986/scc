#include <stdio.h>

int
main() {
#define A 2
#define B 3
#if A > 0
#if B > 0
#define MSG "Both positive"
#else
#define MSG "A positive, B non-positive"
#endif
#else 
#if B > 0
#define MSG "B positive, A non-positive"
#else
#define MSG "Both non-positive"
#endif
#endif
	printf("msg: %s\n", MSG);

#undef A
#undef B
#undef MSG
#define A 2
#define B -3
#if A > 0
#if B > 0
#define MSG "Both positive"
#else
#define MSG "A positive, B non-positive"
#endif
#else 
#if B > 0
#define MSG "B positive, A non-positive"
#else
#define MSG "Both non-positive"
#endif
#endif
	printf("msg: %s\n", MSG);

#undef A
#undef B
#undef MSG
#define A -2
#define B -3
#if A > 0
#if B > 0
#define MSG "Both positive"
#else
#define MSG "A positive, B non-positive"
#endif
#else 
#if B > 0
#define MSG "B positive, A non-positive"
#else
#define MSG "Both non-positive"
#endif
#endif
	printf("msg: %s\n", MSG);

#undef A
#undef B
#undef MSG
#define A -2
#define B 3
#if A > 0
#if B > 0
#define MSG "Both positive"
#else
#define MSG "A positive, B non-positive"
#endif
#else 
#if B > 0
#define MSG "B positive, A non-positive"
#else
#define MSG "Both non-positive"
#endif
#endif
	printf("msg: %s\n", MSG);

	return 0;
}
