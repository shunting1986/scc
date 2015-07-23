#include <stdio.h>

int
main() {
#if defined(C)
#define MSG "C is defined"
#elif defined(B)
#define MSG "B is defined"
#elif defined(A)
#define MSG "A is defined"
#else 
#define MSG "None is defined"
#endif
	printf("msg: %s\n", MSG);

#undef MSG
#define A
#if defined(C)
#define MSG "C is defined"
#elif defined(B)
#define MSG "B is defined"
#elif defined(A)
#define MSG "A is defined"
#else 
#define MSG "None is defined"
#endif
	printf("msg: %s\n", MSG);

#undef MSG
#define B "b"
#if defined(C)
#define MSG "C is defined"
#elif defined(B)
#define MSG "B is defined"
#elif defined(A)
#define MSG "A is defined"
#else 
#define MSG "None is defined"
#endif
	printf("msg: %s\n", MSG);

#undef MSG
#define C 3
#if C > 2
#define MSG "C is defined"
#elif defined(B)
#define MSG "B is defined"
#elif defined(A)
#define MSG "A is defined"
#else 
#define MSG "None is defined"
#endif
	printf("msg: %s\n", MSG);

	return 0;
}
