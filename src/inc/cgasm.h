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

void cgasm_tree(struct syntree *tree);
void cgasm_function_definition(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt);

// cgasm-emit.c
void cgasm_println(struct cgasm_context *ctx, const char *fmt, ...);

// cgasm-symbol.c
void cgasm_add_symbol(struct cgasm_context *ctx, char *id);

#ifdef __cplusplus
}
#endif

#endif
