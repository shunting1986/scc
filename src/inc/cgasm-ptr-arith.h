#ifndef _INC_CGASM_PTR_ARITH_H
#define _INC_CGASM_PTR_ARITH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inc/cgasm-expr-val.h>
struct cgasm_context;

struct expr_val cgasm_handle_ptr_binary_op(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct expr_val rhs);

#ifdef __cplusplus
}
#endif

#endif
