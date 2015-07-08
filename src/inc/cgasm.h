#ifndef _INC_CGASM_H
#define _INC_CGASM_H

#include <stdio.h>
#include <stdarg.h>
#include <inc/syntree.h>
#include <inc/syntree-node.h>

#ifdef __cplusplus
extern "C" {
#endif

struct syntree;
struct cgasm_context;
struct cgasm_func_context;

struct cgasm_context {
	FILE *fp;
	struct cgasm_func_context *func_ctx;
	struct symtab *top_stab; // the top stab

	struct dynarr *str_literals;
};

struct cgasm_func_context {
	int nparam_word; // number of 32bit words for parameter
	int nlocal_word; // number of 32bit words for local variables
	struct cbuf *code_buf;
};

void cgasm_tree(struct syntree *tree);
void cgasm_function_definition(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt);

// cgasm-emit.c
void cgasm_println(struct cgasm_context *ctx, const char *fmt, ...);

// cgasm-symbol.c
void cgasm_add_decl_sym(struct cgasm_context *ctx, char *id);
void cgasm_add_param_sym(struct cgasm_context *ctx, char *id, int ind);
struct symbol *cgasm_lookup_sym(struct cgasm_context *ctx, char *id);

// cgasm-expr.c
struct expr_val cgasm_expression(struct cgasm_context *ctx, struct expression *expr);

// str-literals.c
void cgasm_destroy_str_literals(struct cgasm_context *ctx);
struct expr_val cgasm_register_str_literal(struct cgasm_context *ctx, char *str);
char *get_str_literal_label(int ind, char *buf);

// handle-op.c
struct expr_val cgasm_handle_unary_op(struct cgasm_context *ctx, int tok_tag, struct expr_val operand);
void cgasm_push_val(struct cgasm_context *ctx, struct expr_val val);
struct expr_val cgasm_handle_binary_op(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct expr_val rhs);
struct expr_val cgasm_handle_assign_op(struct cgasm_context *ctx, struct expr_val lhs, struct expr_val rhs, int op);
void cgasm_load_val_to_reg(struct cgasm_context *ctx, struct expr_val val, int reg);
void cgasm_store_reg_to_mem(struct cgasm_context *ctx, int reg, struct expr_val mem);

// cgasm-expr-val.c
#include <inc/cgasm-expr-val.h>

#ifdef __cplusplus
}
#endif

#endif
