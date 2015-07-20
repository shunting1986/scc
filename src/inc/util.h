#ifndef _INC_UTIL_H
#define _INC_UTIL_H

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <inc/dynarr.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
void _panic(const char *fname, int line, char *fmt, ...);
#define panic(str, ...) _panic(__FILE__, __LINE__, str, __VA_ARGS__) 
*/
void _panic(const char *fname, int line, const char *funcname, const char *fmt, ...) __attribute__((noreturn));
#define panic(...) _panic(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

void *mallocz(size_t sz);

#ifndef __cplusplus
typedef int bool;
#define true 1
#define false 0
#endif

#ifdef __cplusplus
}
#endif

#endif
