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
};

struct cgasm_func_context {
	int nparam_word; // number of 32bit words for parameter
	int nlocal_word; // number of 32bit words for local variables
};

void cgasm_tree(struct syntree *tree);
void cgasm_function_definition(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt);

// cgasm-emit.c
void cgasm_println(struct cgasm_context *ctx, const char *fmt, ...);

// cgasm-symbol.c
void cgasm_add_decl_sym(struct cgasm_context *ctx, char *id);
void cgasm_add_param_sym(struct cgasm_context *ctx, char *id, int ind);

// cgasm-expr.c
struct expr_val cgasm_expression(struct cgasm_context *ctx, struct expression *expr);

// cgasm-expr-val.c
struct temp_var {
	int ind; // use (anonymous) local variable as temporary variable right now
};

enum {
	EXPR_VAL_TEMP,
	EXPR_VAL_SYMBOL,
	EXPR_VAL_VOID,
	EXPR_VAL_REGISTER, //
};

struct expr_val {
	int type;
	union {
		struct symbol *sym;
		struct temp_var temp_var;
	};
};

#ifdef __cplusplus
}
#endif

#endif
