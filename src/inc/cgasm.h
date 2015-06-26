#ifndef _INC_CGASM_H
#define _INC_CGASM_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct syntree;
struct cgasm_context;
struct cgasm_func_context;

struct cgasm_context {
	FILE *fp;
	struct cgasm_func_context *func_ctx;
};

void cgasm_tree(struct syntree *tree);

#ifdef __cplusplus
}
#endif

#endif
