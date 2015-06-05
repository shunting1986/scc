#ifndef _INC_UTIL_H
#define _INC_UTIL_H

#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void panic(char *str, ...);

void *mallocz(size_t sz);

#ifdef __cplusplus
}
#endif

#endif
