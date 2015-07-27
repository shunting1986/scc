#define __restrict
#define __THROWNL 
#define __attribute__(xyz) 
typedef int size_t;
#define _G_va_list void *

int printf(const char *s, ...);
extern int vsnprintf (char *__restrict __s, size_t __maxlen,                            
          const char *__restrict __format, _G_va_list __arg)                            
					     __THROWNL __attribute__ ((__format__ (__printf__, 3, 0)));

int
main() {
	printf("hi\n");
	return 0;
}
