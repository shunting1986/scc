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

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

/*
void _panic(const char *fname, int line, char *fmt, ...);
#define panic(str, ...) _panic(__FILE__, __LINE__, str, __VA_ARGS__) 
*/
void _panic(const char *fname, int line, const char *funcname, const char *fmt, ...) __attribute__((noreturn));
#define panic(...) _panic(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

void red(const char *fmt, ...);

void *mallocz(size_t sz);

#ifndef __cplusplus
typedef int bool;
#define true 1
#define false 0
#endif

static inline int is_hex_char(char ch) {
	return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

static inline int get_hex_value_from_char(char ch) {
	assert(is_hex_char(ch));
	if (ch >= '0' && ch <= '9') {
		return ch - '0'; 
	} else if (ch >= 'A' && ch <= 'F') {
		return ch - 'A' + 10;
	} else {
		return ch - 'a' + 10;
	}
}

#ifdef __cplusplus
}
#endif

#endif
