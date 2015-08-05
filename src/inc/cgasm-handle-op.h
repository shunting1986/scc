#ifndef _INC_CGASM_HANDLE_OP_H
#define _INC_CGASM_HANDLE_OP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inc/cgasm-expr-val.h>
#include <inc/symtab.h>

struct local_var_symbol;
struct param_symbol;

struct expr_val cgasm_handle_unary_op(struct cgasm_context *ctx, int tok_tag, struct expr_val operand);
void cgasm_push_val(struct cgasm_context *ctx, struct expr_val val);
struct expr_val cgasm_handle_binary_op(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct expr_val rhs);
struct expr_val cgasm_handle_binary_op_lazy(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct syntreebasenode *rhs);
struct expr_val cgasm_handle_assign_op(struct cgasm_context *ctx, struct expr_val lhs, struct expr_val rhs, int op);
void cgasm_load_val_to_reg(struct cgasm_context *ctx, struct expr_val val, int reg);
void cgasm_store_reg_to_mem(struct cgasm_context *ctx, int reg, struct expr_val mem);
void cgasm_store_reg_to_temp_var(struct cgasm_context *ctx, int reg, struct temp_var temp);
void cgasm_handle_ret(struct cgasm_context *ctx);
void cgasm_test_expr(struct cgasm_context *ctx, struct expr_val val);
struct expr_val cgasm_handle_post_inc(struct cgasm_context *ctx, struct expr_val val);
struct expr_val cgasm_handle_post_dec(struct cgasm_context *ctx, struct expr_val val);
struct expr_val cgasm_handle_pre_inc(struct cgasm_context *ctx, struct expr_val val);
struct expr_val cgasm_handle_pre_dec(struct cgasm_context *ctx, struct expr_val val);
struct expr_val cgasm_handle_index_op(struct cgasm_context *ctx, struct expr_val base_val, struct expr_val ind_val);

struct expr_val cgasm_handle_conditional(struct cgasm_context *ctx, struct expr_val cond, struct dynarr *inner_expr_list, int inner_expr_ind, struct dynarr *or_expr_list, int or_expr_ind, struct expr_val temp_var);

struct expr_val cgasm_handle_ptr_op(struct cgasm_context *ctx, struct expr_val stptr, const char *name);
struct expr_val cgasm_handle_ampersand(struct cgasm_context *ctx, struct expr_val operand);

struct expr_val cgasm_handle_deref_flag(struct cgasm_context *ctx, struct expr_val operand);
int cgasm_get_local_var_offset(struct cgasm_context *ctx, struct local_var_symbol *sym);
int cgasm_get_param_offset(struct cgasm_context *ctx /* unused */, struct param_symbol *sym);
void cgasm_load_addr_to_reg(struct cgasm_context *ctx, struct expr_val val, int reg);
int cgasm_get_temp_var_offset(struct cgasm_context *ctx, struct temp_var temp);
char *cgasm_get_lval_asm_code(struct cgasm_context *ctx, struct expr_val val, char *buf);
const char *size_to_suffix(int size);
void cgasm_extend_reg(struct cgasm_context *ctx, int reg, struct type *type);

#ifdef __cplusplus
}
#endif

#endif
