#ifndef _INC_FP_H
#define _INC_FP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inc/cgasm-expr-val.h>

struct cgasm_context;
struct type;

struct expr_val fp_type_convert(struct cgasm_context *ctx, struct expr_val val, struct type *newtype);

#ifdef __cplusplus
}
#endif

#endif
