#ifndef _INC_FP_H
#define _INC_FP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inc/cgasm-expr-val.h>

struct cgasm_context;
struct type;

struct expr_val fp_type_convert(struct cgasm_context *ctx, struct expr_val val, struct type *newtype);
struct expr_val cgasm_handle_binary_op_fp(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct expr_val rhs);
void cgasm_push_fp_val_to_gstk(struct cgasm_context *ctx, struct expr_val val);
struct expr_val cgasm_handle_assign_op_fp(struct cgasm_context *ctx, struct expr_val lhs, struct expr_val rhs, int op);
void pop_fpstk_to_lval(struct cgasm_context *ctx, struct expr_val dst);

#ifdef __cplusplus
}
#endif

#endif
