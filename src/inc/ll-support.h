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
struct expr_val cgasm_handle_ll_assign_op(struct cgasm_context *ctx, struct expr_val lhs, struct expr_val rhs, int op);
void cgasm_copy_bytes_to_temp(struct cgasm_context *ctx, int from_base_reg, int from_start_off, struct expr_val temp);
void cgasm_copy_bytes(struct cgasm_context *ctx, int from_base_reg, int from_start_off, int to_base_reg, int to_start_off, int size);

#ifdef __cplusplus
}
#endif

#endif
