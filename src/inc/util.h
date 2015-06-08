#ifndef _INC_UTIL_H
#define _INC_UTIL_H

#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
void _panic(const char *fname, int line, char *fmt, ...);
#define panic(str, ...) _panic(__FILE__, __LINE__, str, __VA_ARGS__) 
*/
void _panic(const char *fname, int line, char *fmt, ...);
#define panic(...) _panic(__FILE__, __LINE__, __VA_ARGS__)

void *mallocz(size_t sz);

#ifdef __cplusplus
}
#endif

#endif
