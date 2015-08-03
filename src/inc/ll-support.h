#ifndef _INC_LL_SUPPORT_H
#define _INC_LL_SUPPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inc/cgasm-expr-val.h>

struct cgasm_context;
struct symbol;
struct type;

void cgasm_load_ll_val_to_reg2(struct cgasm_context *ctx, struct expr_val val, int reg1, int reg2);
struct expr_val cgasm_handle_binary_op_ll(struct cgasm_context *ctx, int op, struct expr_val lhs, struct expr_val rhs);
void cgasm_store_reg2_to_ll_temp(struct cgasm_context *ctx, int reg1, int reg2, struct expr_val temp);
void cgasm_push_ll_val(struct cgasm_context *ctx, struct expr_val val);

#ifdef __cplusplus
}
#endif

#endif
