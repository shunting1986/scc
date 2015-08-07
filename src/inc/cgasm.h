#ifndef _INC_CGASM_H
#define _INC_CGASM_H

#include <stdio.h>
#include <stdarg.h>
#include <inc/syntree.h>
#include <inc/syntree-node.h>
#include <inc/symtab.h>
#include <inc/type.h>
#include <inc/util.h>
#include <inc/cgasm-ptr-arith.h>

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

	int nstate_reg;
	int state_reg[8];

	int nasm_label;
	// int const_required;

	// maintain the break continue labels 
	// right now support for, while, do-while
	// TODO support switch for break
	struct intstack *break_label_stk, *continue_label_stk;
};

struct cgasm_func_context {
	const char *name;
	int nparam_word; // number of 32bit words for parameter
	int nlocal_word; // number of 32bit words for local variables
	struct cbuf *code_buf;
};

// cgasm-expr-val.c
#include <inc/cgasm-expr-val.h>

void cgasm_tree(struct syntree *tree);
void cgasm_declaration(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list);
void cgasm_emit_abort(struct cgasm_context *ctx);

void cgasm_push_break_label(struct cgasm_context *ctx, int label_no);
int cgasm_pop_break_label(struct cgasm_context *ctx);
void cgasm_push_continue_label(struct cgasm_context *ctx, int label_no);
int cgasm_pop_continue_label(struct cgasm_context *ctx);

// cgasm-func.c
void cgasm_function_definition(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt);
int func_alloc_space(struct cgasm_func_context *func_ctx, int size);

// cgasm-stmt.c
void cgasm_compound_statement(struct cgasm_context *ctx, struct compound_statement *compound_stmt);
void cgasm_goto_ifcond_cc(struct cgasm_context *ctx, struct condcode *ccexpr, int goto_label, int reverse);
void cgasm_goto_ifcond(struct cgasm_context *ctx, struct expr_val condval, int goto_label, int inverse);

// cgasm-emit.c
void cgasm_println(struct cgasm_context *ctx, const char *fmt, ...);
void cgasm_println_noind(struct cgasm_context *ctx, const char *fmt, ...);

// cgasm-symbol.c
struct symbol *cgasm_add_decl_sym(struct cgasm_context *ctx, char *id, struct type *type, int symbol_flags);
void cgasm_add_param_sym(struct cgasm_context *ctx, char *id, struct type *type);
struct symbol *cgasm_lookup_sym(struct cgasm_context *ctx, char *id);
struct symbol *cgasm_lookup_sym_noabort(struct cgasm_context *ctx, char *id);
void cgasm_push_symtab(struct cgasm_context *ctx);
void cgasm_pop_symtab(struct cgasm_context *ctx);
void cgasm_dump_global_vars(struct cgasm_context *ctx);
struct type *cgasm_get_type_from_type_name(struct cgasm_context *ctx, char *id);
struct type *cgasm_get_union_type_by_name(struct cgasm_context *ctx, const char *name, bool rec);
struct symbol *cgasm_add_struct_type(struct cgasm_context *ctx, const char *name, struct type *type);
struct type *cgasm_get_struct_type_by_name(struct cgasm_context *ctx, bool is_struct, const char *name, bool rec);
struct symbol *cgasm_add_enumerator(struct cgasm_context *ctx, const char *name, int val);
bool check_builtin_symbol(struct cgasm_context *ctx, const char *name, struct expr_val *pval);

// cgasm-expr.c
struct expr_val cgasm_expression(struct cgasm_context *ctx, struct expression *expr);
struct expr_val cgasm_eval_expr(struct cgasm_context *ctx, struct syntreebasenode *rawexpr);
int cgasm_interpret_const_expr(struct cgasm_context *ctx, struct constant_expression *expr);
int cgasm_get_int_const_from_expr(struct cgasm_context *ctx, struct expr_val val);
long long cgasm_get_ll_const_from_expr(struct cgasm_context *ctx, struct expr_val val);
struct expr_val cgasm_assignment_expression(struct cgasm_context *ctx, struct assignment_expression *expr);
struct expr_val cgasm_logical_or_expression(struct cgasm_context *ctx, struct logical_or_expression *expr);
void cgasm_change_array_func_to_ptr(struct cgasm_context *ctx, struct expr_val *pval);

// asm-label.c
void cgasm_destroy_str_literals(struct cgasm_context *ctx);
struct expr_val cgasm_register_str_literal(struct cgasm_context *ctx, const char *str);
char *get_str_literal_label(int ind, char *buf);
void cgasm_dump_string_literals(struct cgasm_context *ctx);
int cgasm_new_label_no(struct cgasm_context *ctx);
void cgasm_emit_jump_label(struct cgasm_context *ctx, int no);
char *get_jump_label_str(int no, char *buf);

// handle-op.c
#include <inc/cgasm-handle-op.h>

// cgasm-decl.c
struct type *parse_type_from_declarator(struct cgasm_context *ctx, struct type *base_type, struct declarator *declarator, char **idret);

// initializer.c
void cgasm_allocate_global_var(struct cgasm_context *ctx, struct global_var_symbol *sym, struct initializer *initializer);

// str-op.c
void cgasm_push_bytes(struct cgasm_context *ctx, int from_base_reg, int from_start_off, int size);
void cgasm_copy_bytes_to_temp(struct cgasm_context *ctx, int from_base_reg, int from_start_off, struct expr_val temp);
void cgasm_copy_bytes(struct cgasm_context *ctx, int from_base_reg, int from_start_off, int to_base_reg, int to_start_off, int size);

#ifdef __cplusplus
}
#endif

#endif
